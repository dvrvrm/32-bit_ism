#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#define REG_NEXTPC 16
#define REG_PC 15
#define R_FLAGS 17
struct word{
char data[33];
};
struct _decoder_output{
char opc[7];
char opd1[12];
char opd2[6];
char offset[9];
char immi[3];
};
struct pipe{
int addr;
struct pipe *next;
};

typedef struct word word;
typedef struct _decoder_output _decoder_output;
typedef struct pipe pipe;
typedef void (*fp) (char *,char *,char *,char *);

word mem[216];
word regfile[32];
int clock;
pipe *_p_head;
pipe *_p_tail;
int pending_bds;
int no_of_ins=0;
int stop_ins=50000;

int bintodec(char *);
void init_sim();
int load_program(char *);
void print(word *);
void set_reg_val(word *,int);
int get_reg_val(int);
void init_pipeline();
void init_clock();
void init_memory(int);
void init_regs(int);
void start_cpu(void);
void start_new_cycle();
void exec_pipeline_stages();
void fetch(int);
char *peek_pipeline(int);
void decode(char *,_decoder_output *);
void execute(_decoder_output *);
void (*instr_handlers[22])(char *,char *,char *,char *);

void opc_jmp_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd1)-1;
set_reg_val(regfile+16,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=dec_im_opd1-1;
set_reg_val(regfile+16,ans);
}
}
pending_bds=2;
}

void opc_je_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
int flag=get_reg_val(17);
if(flag==0){
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd1)-1;
set_reg_val(regfile+15,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=dec_im_opd1-1;
set_reg_val(regfile+15,ans);
}
}
pending_bds=2;
}
}

void opc_jne_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
int flag=get_reg_val(17);
if(flag!=0){
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd1)-1;
set_reg_val(regfile+15,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=dec_im_opd1-1;
set_reg_val(regfile+15,ans);
}
}
pending_bds=2;
}
}

void opc_jg_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
int flag=get_reg_val(17);
if(flag==1){
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd1)-1;
set_reg_val(regfile+16,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=dec_im_opd1-1;
set_reg_val(regfile+16,ans);
}
}
pending_bds=2;
}
}

void opc_jge_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
int flag=get_reg_val(17);
if(flag==0 || flag==1){
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd1)-1;
set_reg_val(regfile+15,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=dec_im_opd1-1;
set_reg_val(regfile+15,ans);
}
}
pending_bds=2;
}
}

void opc_jl_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
int flag=get_reg_val(17);
if(flag==2){
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd1)-1;
set_reg_val(regfile+15,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=dec_im_opd1-1;
set_reg_val(regfile+15,ans);
}
}
pending_bds=2;
}
}

void opc_jle_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
int flag=get_reg_val(17);
if(flag==0 || flag==2){
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd1)-1;
set_reg_val(regfile+15,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=dec_im_opd1-1;
set_reg_val(regfile+15,ans);
}
}
pending_bds=2;
}
}

void opc_push_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
int val_esp=get_reg_val(8);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd1);
set_reg_val(mem+val_esp,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=dec_im_opd1;
set_reg_val(mem+val_esp,ans);
}
}
int new_esp=get_reg_val(8)-1;
set_reg_val(regfile+8,new_esp);
}

void opc_pop_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
int val_esp=get_reg_val(8);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
int val_mem=get_mem_val(mem+val_esp);
ans=val_mem;
set_reg_val(regfile+dec_opd1,ans);
}
}
int new_esp=get_reg_val(8)+1;
set_reg_val(regfile+8,new_esp);
}
  
void opc_mul_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
int val_eax=get_reg_val(0);
ans=val_eax*get_reg_val(dec_opd1);
set_reg_val(regfile+0,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_reg_val(0)*dec_im_opd1;
set_reg_val(regfile+0,ans);
}
}
}

void opc_div_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
int val_eax=get_reg_val(0);
ans=val_eax/get_reg_val(dec_opd1);
set_reg_val(regfile+0,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_reg_val(0)/dec_im_opd1;
set_reg_val(regfile+0,ans);
}
}
}

void opc_add_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
int val_opd2=get_reg_val(dec_opd2);
ans=val_opd2+get_reg_val(dec_opd1);
set_reg_val(regfile+dec_opd2,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd2)+dec_im_opd1;
set_reg_val(regfile+dec_opd2,ans);
}
}
else{
int dec_off=bintodec(offset);
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)+get_reg_val(dec_opd1);
set_reg_val(mem+dec_opd2+dec_off,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)+dec_im_opd1;
set_reg_val(mem+dec_opd2+dec_off,ans);
}
}
if(ans==0){
set_reg_val(regfile+R_FLAGS,0);
}
else if(ans>0){
set_reg_val(regfile+R_FLAGS,1);
}
else{
set_reg_val(regfile+R_FLAGS,2);
}
}

void opc_cmp_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
int val_opd2=get_reg_val(dec_opd2);
ans=val_opd2-get_reg_val(dec_opd1);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd2)-dec_im_opd1;
}
}
else{
int dec_off=bintodec(offset);
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)-get_reg_val(dec_opd1);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)-dec_im_opd1;
}
}
if(ans==0){
set_reg_val(regfile+R_FLAGS,0);
}
else if(ans>0){
set_reg_val(regfile+R_FLAGS,1);
}
else{
set_reg_val(regfile+R_FLAGS,2);
}
}

void opc_sub_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
int val_opd2=get_reg_val(dec_opd2);
ans=val_opd2-get_reg_val(dec_opd1);
set_reg_val(regfile+dec_opd2,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd2)-dec_im_opd1;
set_reg_val(regfile+dec_opd2,ans);
}
}
else{
int dec_off=bintodec(offset);
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)-get_reg_val(dec_opd1);
set_reg_val(mem+dec_opd2+dec_off,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)-dec_im_opd1;
set_reg_val(mem+dec_opd2+dec_off,ans);
}
}
if(ans==0){
set_reg_val(regfile+R_FLAGS,0);
}
else if(ans>0){
set_reg_val(regfile+R_FLAGS,1);
}
else{
set_reg_val(regfile+R_FLAGS,2);
}
}

void opc_mov_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
int val_opd2=get_reg_val(dec_opd2);
ans=get_reg_val(dec_opd1);
set_reg_val(regfile+dec_opd2,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=dec_im_opd1;
set_reg_val(regfile+dec_opd2,ans);
}
}
else{
int dec_off=bintodec(offset);
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd1);
set_reg_val(mem+dec_opd2+dec_off,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=dec_im_opd1;
set_reg_val(mem+dec_opd2+dec_off,ans);
}
}
if(ans==0){
set_reg_val(regfile+R_FLAGS,0);
}
else if(ans>0){
set_reg_val(regfile+R_FLAGS,1);
}
else{
set_reg_val(regfile+R_FLAGS,2);
}
}

void opc_and_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
int val_opd2=get_reg_val(dec_opd2);
ans=val_opd2&get_reg_val(dec_opd1);
set_reg_val(regfile+dec_opd2,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd2)&dec_im_opd1;
set_reg_val(regfile+dec_opd2,ans);
}
}
else{
int dec_off=bintodec(offset);
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)&get_reg_val(dec_opd1);
set_reg_val(mem+dec_opd2+dec_off,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)&dec_im_opd1;
set_reg_val(mem+dec_opd2+dec_off,ans);
}
}
if(ans==0){
set_reg_val(regfile+R_FLAGS,0);
}
else if(ans>0){
set_reg_val(regfile+R_FLAGS,1);
}
else{
set_reg_val(regfile+R_FLAGS,2);
}
}

void opc_xor_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
int val_opd2=get_reg_val(dec_opd2);
ans=val_opd2^get_reg_val(dec_opd1);
set_reg_val(regfile+dec_opd2,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd2)^dec_im_opd1;
set_reg_val(regfile+dec_opd2,ans);
}
}
else{
int dec_off=bintodec(offset);
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)^get_reg_val(dec_opd1);
set_reg_val(mem+dec_opd2+dec_off,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)^dec_im_opd1;
set_reg_val(mem+dec_opd2+dec_off,ans);
}
}
if(ans==0){
set_reg_val(regfile+R_FLAGS,0);
}
else if(ans>0){
set_reg_val(regfile+R_FLAGS,1);
}
else{
set_reg_val(regfile+R_FLAGS,2);
}
}

void opc_or_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans;
int dec_opd2=bintodec(opd2);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
int val_opd2=get_reg_val(dec_opd2);
ans=val_opd2|get_reg_val(dec_opd1);
set_reg_val(regfile+dec_opd2,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_reg_val(dec_opd2)|dec_im_opd1;
set_reg_val(regfile+dec_opd2,ans);
}
}
else{
int dec_off=bintodec(offset);
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)|get_reg_val(dec_opd1);
set_reg_val(mem+dec_opd2+dec_off,ans);
}
else{
int dec_im_opd1=bintodec(opd1);
ans=get_mem_val(dec_opd2+dec_off)|dec_im_opd1;
set_reg_val(mem+dec_opd2+dec_off,ans);
}
}
if(ans==0){
set_reg_val(regfile+R_FLAGS,0);
}
else if(ans>0){
set_reg_val(regfile+R_FLAGS,1);
}
else{
set_reg_val(regfile+R_FLAGS,2);
}
}

void opc_xchange_handler(char *opd1,char *opd2,char *offset,char *immi){
int ans1,ans2;
int dec_opd2=bintodec(opd2);
if(immi[1]=='0'){
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
int val_opd2=get_reg_val(dec_opd2);
ans1=get_reg_val(dec_opd1);
set_reg_val(regfile+dec_opd2,ans1);
ans2=get_reg_val(dec_opd2);
set_reg_val(regfile+dec_opd1,ans2);
}
}
else{
int dec_off=bintodec(offset);
if(immi[0]=='0'){
int dec_opd1=bintodec(opd1);
ans1=get_mem_val(dec_opd2+dec_off)+get_reg_val(dec_opd1);
set_reg_val(mem+dec_opd2+dec_off,ans1);
ans2=get_mem_val(dec_opd2+dec_off);
set_reg_val(regfile+dec_opd1,ans2);
}
}
}

int main(int argc,char *argv[]){
if(argc==4){
sscanf(argv[3],"%d",&stop_ins);
}
init_sim();
int program_start= load_program(argv[1]);
printf("\n");
start_cpu();
return 0;
}

void print(word *l){
int i;
for(i=0;i<32;i++){
printf("---%d------%s------\n",i,(l+i)->data);
}
printf("///////////////////////////////////////////////////////////////////////////////////////////");
}

void dectobin(char *l,int v){
int j=0,i=0,k=32;
if(v<0){
l[0]='1';
}
else
l[0]='0';
for(i=1;i<32;i++){
if((int)(v/pow(2,32-i-1))==0){
l[i]='0';
}
else{
if(v==0)
l[i]='0';
else{
l[i]='1';
int j=pow(2,32-i-1);
v=v%j;
}
}
}
l[i]='\0';
printf("\n");
}

void set_reg_val(word *l,int v){
int i;
dectobin(l->data,v);
}

int get_reg_val(int k){
char *ss=(regfile+k)->data;
int sum=0,i;
for(i=0;i<31;i++){
if((ss[31-i])=='0')
sum=sum;
else{
sum=sum+pow(2,i);
}
}
if(ss[0]=='0')
return sum;
else
return -sum;
}

int get_mem_val(int k){
char *ss=(mem+k)->data;
int sum=0,i;
for(i=0;i<31;i++){
if((*(ss+31-i))=='0')
sum=sum;
else{
sum=sum+pow(2,i);
}
}
if(ss[i]=='0')
return sum;
else
return -sum;
}


void init_sim(void){
init_regs(0);
init_memory(0);
init_clock();
init_pipeline();
}

void init_regs(int k){
if(k==0){
int i;
for(i=0;i<32;i++){
set_reg_val(regfile+i,0);
}
set_reg_val(regfile+8,100);
}
}

void init_memory(int k){
if(k==0){
int i;
for(i=0;i<216;i++){
set_reg_val(mem+i,0);
}
}
}

void init_clock(){
clock=0;
}

void init_pipeline(void){
_p_head=malloc(sizeof(pipe));
_p_head->addr=-1;
_p_head->next=NULL;
_p_tail=_p_head;
}

int load_program(char *fname){
FILE *ifp;
char ch;
int i=0,j=0;
ifp= fopen(fname,"r");
while((ch=fgetc(ifp))!=EOF){
if(ch=='\n' && j==0)
continue;
if(ch=='\n'){
j++;
ch='\0';
mem[i].data[j]=ch;
i++;
j=0;
}
else{
mem[i].data[j]=ch;
j++;
}
}
if(j!=0){
mem[i].data[j]='\0';
}
set_reg_val(mem+i,0);
fclose(ifp);
return 0;
}

void start_cpu(void){
int RESET_VEC_ADDR=0;
set_reg_val(regfile+16,RESET_VEC_ADDR);     //NEXT_PC
set_reg_val(regfile+15,get_reg_val(REG_NEXTPC)-2);   //PC
pending_bds =0 ;
while(1){
if(no_of_ins-1>stop_ins){
printf("\n\npc is %d\n",get_reg_val(15));
printf("\n\nregister file is\n");
print(regfile);
printf("\n\nmemory is\n");
print(mem);
exit(0);
}
start_new_cycle();
exec_pipeline_stages();
}
}

void start_new_cycle(){
clock++;
}

void exec_pipeline_stages(void){
_decoder_output cur_decoder_output;
static _decoder_output prev_decoded_instr;
fetch(get_reg_val(REG_NEXTPC));
decode(peek_pipeline(1),&cur_decoder_output);
execute(&prev_decoded_instr);
prev_decoded_instr = cur_decoder_output;
}

void enpipe(int instr_addr){
if(_p_head->addr==-1)
_p_head->addr=instr_addr;
else{
if(_p_head->next==NULL){
_p_head->next=malloc(sizeof(pipe));
_p_head->next->addr=_p_head->addr;
_p_head->addr=instr_addr;
}
else{ 
if(_p_head->next->next==NULL){
_p_head->next->next=malloc(sizeof(pipe));
}
_p_head->next->next->addr=_p_head->next->addr;
_p_head->next->addr=_p_head->addr;
_p_head->addr=instr_addr;
}
}
}
char *peek_pipeline(int n){
pipe *temp;
temp=_p_head;
while(n){
if(temp->next!=NULL){
temp=temp->next;
return (mem+temp->addr)->data;
}
else{
return NULL;
}
n--;
}
}

void fetch(int instr_addr){
no_of_ins++;
enpipe(instr_addr);
}
get_opcode(char *in,_decoder_output *out){
int i;
for(i=0;i<6;i++){
out->opc[i]=in[i];
}
out->opc[i]='\0';
}
get_operands(char *in,_decoder_output *out){
int i;
for(i=0;i<11;i++){
out->opd1[i]=in[i+6];
}
out->opd1[i]='\0';
for(i=0;i<5;i++){
out->opd2[i]=in[i+17];
}
out->opd2[i]='\0';
for(i=0;i<8;i++){
out->offset[i]=in[i+22];
}
out->offset[i]='\0';
for(i=0;i<2;i++){
out->immi[i]=in[i+30];
}
out->immi[i]='\0';
}

void decode( char *instr, _decoder_output *out){
if(instr){
get_opcode(instr,out);
get_operands(instr,out);
}
else{
out->opc[0] = '\0';
}
set_reg_val(regfile+REG_NEXTPC,get_reg_val(REG_NEXTPC)+1);
}

int bintodec(char *binary){
int i=0,sum=0,j=0;
while(binary[i]!='\0')
i++;
while(binary[j]!='\0'){
if(binary[j]=='1'){
sum=sum+pow(2,i-j-1);
}
j++;
}
return sum;
}

void execute(_decoder_output *decoded_instr){
instr_handlers[0]=opc_add_handler;
instr_handlers[1]=opc_sub_handler;
instr_handlers[2]=opc_mov_handler;
instr_handlers[3]=opc_and_handler;
instr_handlers[4]=opc_xor_handler;
instr_handlers[5]=opc_or_handler;
instr_handlers[6]=opc_cmp_handler;

instr_handlers[8]=opc_xchange_handler;
instr_handlers[9]=opc_push_handler;
instr_handlers[10]=opc_pop_handler;
instr_handlers[11]=opc_mul_handler;
instr_handlers[12]=opc_mul_handler;
instr_handlers[13]=opc_div_handler;
instr_handlers[14]=opc_div_handler;
instr_handlers[15]=opc_jmp_handler;
instr_handlers[16]=opc_je_handler;
instr_handlers[17]=opc_jne_handler;
instr_handlers[18]=opc_jg_handler;
instr_handlers[19]=opc_jge_handler;
instr_handlers[20]=opc_jl_handler;
instr_handlers[21]=opc_jle_handler;
instr_handlers[22]=opc_jmp_handler;
if(clock>2){
int dec_opc=bintodec(decoded_instr->opc);
if(dec_opc==63){
printf("\n\npc is %d\n",get_reg_val(15));
printf("\n\nregister file is\n");
print(regfile);
printf("\n\nmemory is\n");
print(mem);
printf("exiting");
exit(0);
}
instr_handlers[dec_opc](decoded_instr->opd1,decoded_instr->opd2,decoded_instr->offset,decoded_instr->immi);
}
switch(pending_bds){
case 2:
set_reg_val(regfile+REG_PC,get_reg_val(REG_PC)+1);
pending_bds--;
break;
case 1:
set_reg_val(regfile+REG_PC,get_reg_val(REG_NEXTPC)-2);
pending_bds--;
break;
case 0:
set_reg_val(regfile+REG_PC,get_reg_val(REG_PC)+1);
break;
}
}
