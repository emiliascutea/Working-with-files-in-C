#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdbool.h>

typedef struct{
    int * no_sections;
    char * sect_name;
    int * sect_type;
    int * sect_offset;
    int * sect_size;
} Section_Header;

void list_directory_contents(char * input, char * name_starts_with, bool recursive, bool has_perm_write, bool * ok){
    DIR * directory = opendir(input); // open the directory of the given input path

    if(directory == NULL){ // return 0 if the directory could not be opened
        perror("ERROR \n invalid directory path \n");
        return;
    }

    struct dirent * directoryEntry; // create an instance of dirent for the sub-directories of the input directory
    char* path = (char*)malloc(sizeof(char)*PATH_MAX);
    while ((directoryEntry = readdir(directory)) != 0){ // while we can read subdirectories from the input directory

        if(directoryEntry-> d_type != DT_DIR){ // if the entry is not a directory

            if(name_starts_with != NULL){ // if option name_starts_with is enabled

                if(strstr(directoryEntry->d_name, name_starts_with) == directoryEntry->d_name){ // check if the file has the same name as the given one

                    sprintf(path, "%s/%s", input, directoryEntry->d_name); // get file's path                  
  
                    if(*ok == false){
                        *ok = true;
                        printf("SUCCESS\n"); // print SUCCESS before the first printed path
                    }

                    printf("%s\n",path); // print its path                                      
                } 
            } 
            else if (has_perm_write == true){ // if option has_perm_write is `enabled

                sprintf(path, "%s/%s", input, directoryEntry->d_name); // get file's path

                if(access(path,W_OK) == 0){
                    if(*ok == false){
                        *ok = true;
                        printf("SUCCESS\n"); // print SUCCESS before the first printed path
                    }
                    printf("%s\n",path); // print its path
                }    
            }
            else{                
                sprintf(path, "%s/%s", input, directoryEntry->d_name); // get file's path
                    
                if(*ok == false){
                    *ok = true;
                    printf("SUCCESS\n"); // print SUCCESS before the first printed path
                }
                printf("%s\n",path); // print its path
            }
        }         
        
        // if the entry is a directory and does not have the name . or ..
        if(directoryEntry->d_type == DT_DIR && strcmp(directoryEntry->d_name,".")!=0 && strcmp(directoryEntry->d_name,"..")!=0){

            if(name_starts_with != NULL){ // if the option name_starts_with is enabled

                if(strstr(directoryEntry->d_name, name_starts_with) == directoryEntry->d_name){ // check if the directory has the same name as the given one

                    sprintf(path, "%s/%s", input, directoryEntry->d_name); // get file's path                  

                    if(*ok == false){
                        *ok = true;
                        printf("SUCCESS\n"); // print SUCCESS before the first printed path
                    }
                    printf("%s\n",path); // print its path
                }                
            }
            else if(has_perm_write == true){ // if option has_perm_write is enabled
                sprintf(path, "%s/%s", input, directoryEntry->d_name); // get file's path

                if(access(path,W_OK) == 0){
                    if(*ok == false){
                        *ok = true;
                        printf("SUCCESS\n"); // print SUCCESS before the first printed path
                    }
                    printf("%s\n",path); // print its path
                } 
            }
            else{                
                sprintf(path, "%s/%s", input, directoryEntry->d_name); // get file's path
                    
                if(*ok == false){
                    *ok = true;
                    printf("SUCCESS\n"); // print SUCCESS before the first printed path
                }
                printf("%s\n",path); // print its path
            }

            if(recursive == true){ // if option recursive is enabled, call the function again for this sub-directory to print all its contents
                list_directory_contents(path, name_starts_with, recursive, has_perm_write, ok);
            } 
        }
    }
    if(*ok == false){
        *ok = true;
        printf("SUCCESS\n"); // print SUCCESS
    } 
    free(path);
    free(directory);
    free(directoryEntry);   
}

Section_Header *  parse_file(char * input, bool parseFunction){

    int fd = open(input, O_RDONLY);

    if(fd < 0){
        if(parseFunction == true){
            printf("ERROR \n Cannot open file.\n");
        }
        return NULL;
    }
    else{
        char * magic = (char*)malloc(sizeof(char));
        lseek(fd,0,SEEK_SET);
        read(fd,magic,1);
        if(strcmp(magic,"R") != 0){
            if(parseFunction == true){
                printf("ERROR \n wrong magic");
            }
            return NULL;
        }        
        else{
            char * header_size = (char*)malloc(sizeof(char)*2);
            int * version_size = (int*)malloc(sizeof(int)*4);
            int * no_sections = (int*)malloc(sizeof(int));

            read(fd,header_size,2);

            read(fd,version_size,4);

            if(*version_size < 60 || *version_size> 109){
                if(parseFunction == true){
                    printf("ERROR \n wrong version");
                }
                return NULL;
            }

            read(fd, no_sections,1);            
    
            if(*no_sections < 5 || *no_sections > 12){
                if(parseFunction == true){
                    printf("ERROR \n wrong sect_nr");
                }
                return NULL;
            }

            Section_Header * section_header = (Section_Header*) malloc(sizeof(Section_Header) * (*no_sections));

            section_header->no_sections=no_sections;
            
            for(int index = 0; index < *no_sections; index ++){

                section_header[index].sect_name = (char*)malloc(sizeof(char)*10);

                read(fd, section_header[index].sect_name, 10);

                section_header[index].sect_type = (int*)malloc(sizeof(int));

                read(fd, section_header[index].sect_type, 1);            

                if (*section_header[index].sect_type != 86 && *section_header[index].sect_type != 93 && *section_header[index].sect_type != 76 && *section_header[index].sect_type != 69){
                    if(parseFunction == true){
                        printf("ERROR \n wrong sect_types");
                    }
                    return NULL;
                }

                section_header[index].sect_offset = (int*)malloc(sizeof(int)*4);

                read(fd, section_header[index].sect_offset, 4);

                section_header[index].sect_size = (int*)malloc(sizeof(int)*4);

                read(fd, section_header[index].sect_size, 4);
            }

            if(parseFunction == true){
                printf("SUCCESS\n");
                printf("version=%d\n", *version_size);
                printf("nr_sections=%d \n", *no_sections);

                for(int i = 0; i < *no_sections; i++){
                    printf("section%d: ", i+1);
                    printf("%s ", section_header[i].sect_name);
                    printf("%d ", *section_header[i].sect_type);
                    printf("%d\n", *section_header[i].sect_size);
                }
            }
            free(magic);
            free(version_size);

            return section_header;             
        }
    }
    return NULL;
}

int extract(char * input, int sect_nr, int line_nr, bool extractFunction){

     int fd = open(input, O_RDONLY);

    if(fd < 0){
        printf("ERROR \n invalid file \n");
        return 0;
    }
    Section_Header * section_header = parse_file(input, false);

    if(section_header != NULL){
        if(sect_nr > *section_header->no_sections){
            printf("ERROR \n invalid section");
            return 0;
        }
        lseek(fd,0,SEEK_SET);
        int * jump = section_header[sect_nr-1].sect_offset;
        int count = 1;
        lseek(fd,*jump,SEEK_SET);

        char * line = (char*) malloc(sizeof(char) * PATH_MAX);
        char * foundLine = (char*)malloc(sizeof(char)*PATH_MAX);

        bool found = false;
        int lineIndex=-1;
        int size = PATH_MAX;
        int reallocCount = 1;

        while(found == 0){
            read(fd,line,PATH_MAX);
            for(int index=0;index<PATH_MAX && found == 0;index++){
                if(line[index] == '\n'){
                    count++;
                }

                if(count == line_nr ){
                    if(lineIndex == -1){
                        lineIndex=0;
                    }
                    if(lineIndex == size-1){
                        reallocCount++;
                        size = reallocCount * PATH_MAX;
                        foundLine = realloc(foundLine, size);
                    }

                    foundLine[lineIndex] = line[index];
                    lineIndex++;
                }
                if(count == line_nr + 1){
                    if(found == false){
                        found = true;
                        if(extractFunction == true){
                            printf("SUCCESS\n");
                        }
                    }
                    if(extractFunction == true){
                        for(int index = lineIndex -2; index>=0; index--){
                            printf("%c", foundLine[index]);
                        }
                        printf("\n");
                    }                  
                }
            }
        }        
        if(found == false){
            printf("ERROR \n invalid line");
            return 0;
        }

        for(int i=0;i<*section_header->no_sections;i++){
            free(section_header[i].sect_name);
            free(section_header[i].sect_offset);
            free(section_header[i].sect_size);
            free(section_header[i].sect_type);
        }        
        free(section_header);
    }
    return 1;
}

void findall(char * input, bool * found){

   DIR * directory = opendir(input); // open the directory of the given input path

    if(directory == NULL){ // return 0 if the directory could not be opened
        return;
    }

    struct dirent * directoryEntry; // create an instance of dirent for the sub-directories of the input directory
    char * path = (char*)malloc(sizeof(char)*PATH_MAX);

    while ((directoryEntry = readdir(directory)) != 0){ // while we can read subdirectories from the input directory

        sprintf(path, "%s/%s", input, directoryEntry->d_name); // get file's path   

        if(directoryEntry-> d_type != DT_DIR ){

            Section_Header * entryFile =(Section_Header*)malloc(sizeof(Section_Header));
            entryFile = parse_file(path, false);

            if(entryFile !=  NULL){
                bool valid = false;
                for(int section = 1; section <= *entryFile->no_sections; section++){
                    if(extract(path,section,14,0) == 1){
                        valid = true;                        
                        section = *entryFile->no_sections + 1;
                    }
                }
                if(valid == true){
                    if(*found == false){
                        *found = true;
                        printf("SUCCESS\n");
                    }
                    printf("%s\n",path);
                }
            }
        }
        if(directoryEntry->d_type == DT_DIR && strcmp(directoryEntry->d_name,".")!=0 && strcmp(directoryEntry->d_name,"..")!=0){
            findall(path,found);   
        }
        
    }
    if(*found == false){
        *found = true;
        printf("SUCCESS\n");
    }
}

int main(int argc, char **argv){
    bool ok = false;
    char* path;
    char* name_starts_with = NULL;
    bool recursive = false;
    bool has_perm_write = false;
    char* arg1;
    char* arg2;
    char* arg3;
    char* arg4;
    char* arg5;
    int sect_nr;
    int line_nr;
    bool found = false;
    if(argc >= 2){
        if(strcmp(argv[1], "variant") == 0){
            printf("12741\n");
            return 0;
        }
        else if(strcmp(argv[1],"list") == 0){
            if(argc >= 3){
                if(strcmp(argv[2],"recursive") == 0){
                    recursive = true;
                }
                else if(strcmp(argv[2],"has_perm_write") == 0){
                    has_perm_write = true;
                }
                else{
                    arg2=strtok(argv[2],"=");
                    if(strcmp(arg2,"path") == 0){
                        path=strtok(NULL,"=");
                    }
                    else if(strcmp(arg2,"name_starts_with") == 0){
                        name_starts_with=strtok(NULL,"=");
                    }
                }

                if(argc >= 4){

                    if(strcmp(argv[3],"recursive") == 0){
                        recursive = true;
                    }
                    else if(strcmp(argv[3],"has_perm_write") == 0){
                        has_perm_write = true;
                    }
                    else{
                        arg3=strtok(argv[3],"=");
                        if(strcmp(arg3,"path") == 0){
                            path=strtok(NULL,"=");
                        }
                        else if(strcmp(arg3,"name_starts_with") == 0){
                            name_starts_with=strtok(NULL,"=");
                        }
                    }

                    if(argc >= 5){

                        if(strcmp(argv[4],"recursive") == 0){
                            recursive = true;
                        }
                        else if(strcmp(argv[4],"has_perm_write") == 0){
                            has_perm_write = true;
                        }
                        else{
                            arg4=strtok(argv[4],"=");
                            if(strcmp(arg4,"path") == 0){
                                path=strtok(NULL,"=");
                            }
                            else if(strcmp(arg4,"name_starts_with") == 0){
                                name_starts_with=strtok(NULL,"=");
                            }
                        } 

                        if(argc >= 6){

                            if(strcmp(argv[5],"recursive") == 0){
                                recursive = true;
                            }
                            else if(strcmp(argv[5],"has_perm_write") == 0){
                                has_perm_write = true;
                            }
                            else{
                                arg5=strtok(argv[5],"=");
                                if(strcmp(arg5,"path") == 0){
                                    path=strtok(NULL,"=");
                                }
                                else if(strcmp(arg5,"name_starts_with") == 0){
                                    name_starts_with=strtok(NULL,"=");
                                }
                            }                
                        }                 
                    }                
                } 
                list_directory_contents(path, name_starts_with, recursive, has_perm_write, &ok);                        
            }    
            return 0;       
        }
        else if(strcmp(argv[1],"parse") == 0 || strcmp(argv[2],"parse") == 0){
            if(strcmp(argv[1],"parse") == 0){
                arg2=strtok(argv[2],"=");
                if(strcmp(arg2,"path") == 0){
                    path=strtok(NULL,"=");
                }
            }
            else if(strcmp(argv[2],"parse") == 0){
                arg2=strtok(argv[1],"=");
                if(strcmp(arg2,"path") == 0){
                    path=strtok(NULL,"=");
                }
            }
            parse_file(path, true);
            return 0;
        }
        if(argc==5){
            if(strcmp(argv[1],"extract") == 0 || strcmp(argv[2],"extract") == 0 || strcmp(argv[3],"extract") == 0 || strcmp(argv[4],"extract") == 0){
            if(strcmp(argv[1],"extract") == 0){
                arg2=strtok(argv[2],"=");
                if(strcmp(arg2,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg2,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg2,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }

                arg3=strtok(argv[3],"=");
                if(strcmp(arg3,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg3,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg3,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }

                arg4=strtok(argv[4],"=");
                if(strcmp(arg4,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg4,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg4,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }
            }
            else if(strcmp(argv[2],"extract") == 0){

                arg1=strtok(argv[1],"=");
                if(strcmp(arg1,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg1,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg1,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }

                arg3=strtok(argv[3],"=");
                if(strcmp(arg3,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg3,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg3,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }

                arg4=strtok(argv[4],"=");
                if(strcmp(arg4,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg4,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg4,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }
            }

            else if(strcmp(argv[3],"extract") == 0){

                arg1=strtok(argv[1],"=");
                if(strcmp(arg1,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg1,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg1,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }

                arg2=strtok(argv[2],"=");
                if(strcmp(arg2,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg2,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg2,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }

                arg4=strtok(argv[4],"=");
                if(strcmp(arg4,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg4,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg4,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }
            }
            else if(strcmp(argv[4],"extract") == 0){

                arg1=strtok(argv[1],"=");
                if(strcmp(arg1,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg1,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg1,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }

                arg2=strtok(argv[2],"=");
                if(strcmp(arg2,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg2,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg2,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }

                arg3=strtok(argv[3],"=");
                if(strcmp(arg3,"path") == 0){
                    path=strtok(NULL,"=");
                }
                else if(strcmp(arg3,"section") == 0){
                    sect_nr=atoi(strtok(NULL,"="));
                }
                else if(strcmp(arg3,"line") == 0){
                    line_nr=atoi(strtok(NULL,"="));
                }
            }
            extract(path,sect_nr,line_nr, 1);
            return 0;
        }

        }
        if(argc==3){
            if(strcmp(argv[1],"findall") == 0 || strcmp(argv[2],"findall") == 0){

                if(strcmp(argv[1],"findall") == 0){
                    arg2=strtok(argv[2],"=");
                    if(strcmp(arg2,"path") == 0){
                        path=strtok(NULL,"=");
                    }
                }
                else if(strcmp(argv[2],"findall") == 0){
                    arg1=strtok(argv[1],"=");
                    if(strcmp(arg1,"path") == 0){
                        path=strtok(NULL,"=");
                    }
                }
                findall(path, &found);
            }   
        }            
    }    
    return 0;
}