
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>

typedef struct node_t {
        struct node_t *left, *right;
        int freq;
        char c;
} *node;
int n_nodes = 0, qend = 1;
struct node_t pool[256] = {{0}};
node qqq[255], *q = qqq-1;
char buf[1024];
void import_table(FILE *fp_table, unsigned int *freq){
        char c;
        int i = 0;
        while((c=fgetc(fp_table))!=EOF){
                freq[i++] = (unsigned char)c;
        }
        for (i = 0; i< 128; i++)
                if (freq[i]) qinsert(new_node(freq[i], i, 0, 0));
        while (qend> 2)
                qinsert(new_node(0, 0, qremove(), qremove()));
}



void decode(FILE *fp_huffman,FILE *fp_out)
{
        int i=0,lim=0,j=0;
        char c;
        node n = q[1];
        fscanf(fp_huffman,"%d",&lim);
        fseek(fp_huffman,1,SEEK_CUR);
        printf("Decoded : \n");
        for(i=0; i<lim; i++){
                if(j==0)
                        c = fgetc(fp_huffman);
                if(c&128)       n = n->right;
                else            n = n->left;
                if(n->c){
                        putchar(n->c);
                        fputc(n->c,fp_out);
                        n = q[1];
                }
                c = c<<1;
                if(++j>7)
                        j = 0;
        }
        putchar('\n');
        if (q[1] != n) printf("garbage input\n");
}
int main(int argc, char* argv[]){
        FILE *fp_table,*fp_huffman,*fp_out;
        char file_name[50]={0},temp[50]={0};
        unsigned int freq[128] = {0};
        int shmid;
        char *shm_ptr;
        shmid=shmget((key_t)5900,sizeof(char),0666|IPC_CREAT);
        printf("Key of shared memory shmid1:%d\n",shmid);
        shm_ptr = (char*)shmat(shmid, NULL, 0);
        printf("Please enter the file to be compressed\t: ");
        scanf("%s",file_name);
        printf("Process attached at %p\n",shm_ptr);
        printf("Text file recieved from  shared memory:%d\n\n",(char *)shm_ptr);
       if (shm_ptr == (char*)-1) {
        printf("ERROR: Failed to attach shared memory segment.\n");
        return 0;
    }
        if (shmid< 0) {
        printf("ERROR: Failed to create shared memory segment.\n");
        return 0;
    }
        system("clear");
        if( argc == 2 ) {
                strcpy(file_name,argv[1]);
                if(strstr(file_name,"huffman") == NULL){
                        printf("\nERROR:wrong file format!\n");
                        return 0;
                }
        }
        else if( argc> 2 ) {
                printf("Too many arguments supplied.\n");
        }
        else {
                if(strstr(file_name,"huffman") == NULL){
                        printf("\nERROR:wrong file format!\n");
                        return 0;
                }
        }
        if((fp_huffman = fopen(file_name,"r"))==NULL){
                printf("\nERROR: No such file\n");
                return 0;
        }
        strcat(file_name,".table");
        if((fp_table = fopen(file_name,"r"))==NULL){
                printf("\nERROR: Frequency table cannot be found\n");
                return 0;
        }
        import_table(fp_table,freq);
        *strstr(file_name,".huffman") = '\0';
        strcpy(temp,"mkdir ");
        strcat(temp,file_name);
        system(strcat(temp,".decoded"));
        strcpy(temp,"./");
        strcat(temp,file_name);
        strcat(temp,".decoded/");
        if((fp_out = fopen(strcat(temp,file_name),"w"))==NULL){
                printf("ERROR:Creating decoded file failed\n");
                return 0;
        }
        decode(fp_huffman,fp_out);

        fclose(fp_huffman);
        fclose(fp_table);
        fclose(fp_out);
        return 0;
}
