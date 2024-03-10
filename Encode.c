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
char *code[128] = {0}, buf[1024];
int input_data=0,output_data=0;
node new_node(int freq, char c, node a, node b)
{
        node n = pool + n_nodes++;
        if (freq != 0){
                n->c = c;
                n->freq = freq;
        }
        else {
              n->left = a, n->right = b;
                n->freq = a->freq + b->freq;
        }
        return n;
}

void qinsert(node n)
{
        int j, i = qend++;
        while ((j = i / 2)) {
                if (q[j]->freq<= n->freq) break;
                q[i] = q[j], i = j;
        }
        q[i] = n;
}
node qremove()
{
        int i, l;
        node n = q[i = 1];
        if (qend< 2) return 0;
        qend--;
        while ((l = i * 2) <qend) {
                if (l + 1 <qend&& q[l + 1]->freq< q[l]->freq) l++;
                q[i] = q[l], i = l;
        }
        q[i] = q[qend];
        return n;
}



void build_code(node n, char *s, int len)
{
        static char *out = buf;
        if (n->c) {
                s[len] = 0;
                strcpy(out, s);
                code[(int)n->c] = out;
                out += len + 1;
                return;
        }
        s[len] = '0'; build_code(n->left,  s, len + 1);
        s[len] = '1'; build_code(n->right, s, len + 1);
}

void import_file(FILE *fp_in, unsigned int *freq){
        char c,s[16]={0};
        int i = 0;
        printf("File Read:\n");
        while((c=fgetc(fp_in))!=EOF){
                freq[(int)c]++;
                putchar(c);
        }
        for (i = 0; i< 128; i++)
                if (freq[i]) qinsert(new_node(freq[i], i, 0, 0));
        while (qend> 2)
                qinsert(new_node(0, 0, qremove(), qremove()));
        build_code(q[1], s, 0);
}
void encode(FILE* fp_in, FILE* fp_out, unsigned int *freq )
{

        char in,c,temp[20] = {0};
        int i,j=0,k=0,lim=0;
        rewind(fp_in);
        for(i=0; i<128; i++){
                if(freq[i])     lim += (freq[i]*strlen(code[i]));
        }
        output_data = lim;
        fprintf(fp_out,"%04d\n",lim);
        printf("\nEncoded:\n");
        for(i=0; i<lim; i++){
                if(temp[j] == '\0'){
                        in = fgetc(fp_in);
                        strcpy(temp,code[in]);
                        printf("%s",code[in]);
                        j = 0;
                }
                if(temp[j] == '1')
                        c = c|(1<<(7-k));
                else if(temp[j] == '0')
                        c = c|(0<<(7-k));
                else
                        printf("ERROR: Wrong input!\n");
                k++;
                j++;
                if(((i+1)%8 == 0) || (i==lim-1)){
                        k = 0;
                        fputc(c,fp_out);
                        c = 0;
                }
        }
        putchar('\n');
}
void print_code(unsigned int *freq){
        int i;
        printf("\n---------CODE TABLE---------\n----------------------------\nCHAR  FREQ  CODE\n----------------------------\n");
        for(i=0; i<128; i++){
                if(isprint((char)i)&&code[i]!=NULL&&i!=' ')
                        printf("%-4c  %-4d  %16s\n",i,freq[i],code[i]);
                else if(code[i]!=NULL){
                        switch(i){
                                case '\n':
                                        printf("\\n  ");
                                        break;
                                case ' ':
                                        printf("\' \' ");
                                        break;
                                case '\t':
                                        printf("\\t  ");
                                        break;
                                default:
                                        printf("%0X  ",(char)i);
                                        break;
                        }
                        printf("  %-4d  %16s\n",freq[i],code[i]);
                }
        }
        printf("----------------------------\n");
}

int main(int argc, char* argv[]){
        FILE *fp_in, *fp_out;
        char file_name[50]={0};
        unsigned int freq[128] = {0},i;
        int shmid;
        char *shm_ptr;
        shmid=shmget((key_t)5900,sizeof(char),0666|IPC_CREAT);
        printf("Key of shared memory shmid1:%d\n",shmid);
        if (shmid< 0) {
        printf("ERROR: Failed to create shared memory segment.\n");
        return 0;
    }
        shm_ptr = (char*)shmat(shmid, NULL, 0);
        printf("Please enter the file to be compressed\t: ");
        scanf("%s",file_name);
        shm_ptr=file_name;
        printf("Process attached at %p\n",shm_ptr);
        printf("Text file sent to shared memory:%d\n\n",(char *)shm_ptr);
       if (shm_ptr == (char*)-1) {
        printf("ERROR: Failed to attach shared memory segment.\n");
        return 0;
    }
        if((fp_in = fopen(file_name,"r"))==NULL){
                printf("\nERROR: No such file\n");
                return 0;
        }
        import_file(fp_in,freq);
        print_code(freq);
        strcat(file_name,".huffman");
        fp_out = fopen(file_name,"w");
        encode(fp_in,fp_out,freq);
        fclose(fp_in);
        fclose(fp_out);
        strcat(file_name,".table");
        fp_out = fopen(file_name,"w");
        for(i=0; i<128; i++){
                fprintf(fp_out,"%c",(char)freq[i]);
        }
        for(i=0; i<128; i++)    input_data += freq[i];
        fclose(fp_out);
        printf("\nInput Text file size:\t\t%dkb\n",input_data);
        output_data = (output_data%8)? (output_data/8)+1 : (output_data/8);
        printf("Compressed text file size:\t\t%dkb\n",output_data);

        printf("\nCompression ratio:\t%.2f%%\n\n\n",((double)(input_data-output_data)/input_data)*100);
        return 0;
}
