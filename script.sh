echo hello $1
cat $1 | sed -e 's/inc /add $1,/' -e 's/dec /sub $1,/g' -e 's/%esp/10001/g' -e 's/pc/01111/g' -e 's/%eax/00000/g' -e 's/%ebx/00001/g' -e 's/%ecx/00010/g' -e 's/%edx/00011/g' >> out.txt

cat out.txt | awk -F, 'BEGIN{ FS=" "}
{OFS=",";print $1,$2}
' >> awkout1.txt

cat awkout1.txt | awk -F, '
BEGIN{FS=",";OFS=","}
/:/{ print "--",$1;}
' >> awkout2.txt

cat awkout1.txt >> awkout2.txt


cat awkout2.txt | awk -F, '
BEGIN{ FS=","; OFS=",";i=0;y=0}
/--/{ 
var[i]=$2;
i++;
}
$2==":"{
for(j=0;j<i;j++){
if(var[j]==$1){
print "-",var[j],NR-i-y;
y++;
}
}
}
' >> awkout3.txt

cat awkout1.txt >> awkout3.txt


cat awkout3.txt | awk -F, '
function check(value){
for(j=0;j<i;j++){
if(lab[j]==$2)
print $1,"$"val[j];
}
}

BEGIN{OFS=",";i=0}
$1=="-"{lab[i]=$2;val[i]=$3;i++;fl=0}
/jmp/{check($2);fl=1}
/je/{check($2);fl=1;}
/jne/{check($2);fl=1}
/jg/{check($2);fl=1}
/jge/{check($2);fl=1}
/jl/{check($2);fl=1}
/jle/{check($2);fl=1}
/call/{check($2);fl=1;}
$1!="-"{
if(fl==0 && $2!=":" && $1!="call")
print $0
}
fl=0;


' >> awkout4.txt




cat awkout4.txt | awk -F, '
function binconvert(var1)
{
	for(i=0;i<=23;i++)
        {
               bin[i]=var1%2;
               var1=var1/2;
               if((y=index(var1,"."))>0)
               {
                   var1=substr(var1,1,y-1);
                   if(var1==0)
                   {
                      var1=0;
                   }
                }
        }
}

BEGIN{ FS=",";twoop_d=0}
/add/{ opcode="000000"; op_d=2;}
/sub/{ opcode="000001"; op_d=2;}
/mov/{ opcode="000010"; op_d=2;}
/and/{ opcode="000011"; op_d=2;}
/xor/{ opcode"=000100"; op_d=2;}
/or/{ opcode="000101"; op_d=2;}
/cmp/{ opcode="000110"; op_d=2;}
/test/{ opcode="000111"; op_d=2;}
/xchange/{ opcode="001000"; op_d=2;}

/push/{ opcode="001001"; op_d=1;}
/pop/{ opcode="001010";op_d=1;}
/mull/{ opcode="001011";op_d=1;}
/imul/{ opcode="001100"; op_d=1;}
/div/{ opcode="001101"; op_d=1;}
/idiv/{ opcode="001110"; op_d=1;}
/jmp/{ opcode="001111"; op_d=1;}
/je/{ opcode="010000"; op_d=1;}
/jne/{ opcode="010001"; op_d=1;}
/jg/{ opcode="010010"; op_d=1;}
/jge/{ opcode="010011"; op_d=1;}
/jl/{ opcode="010100"; op_d=1;}
/jle/{ opcode="010101"; op_d=1;}
/call/{ opcode="010110"; op_d=1;}

/leave/{print "11111111111111111111111111111111";}


op_d==1{
OFS="";
if((x=index($2,"$"))>0){
var1=substr($2,2,length($2));
binconvert(var1);
print opcode,bin[10]bin[9]bin[8]bin[7]bin[6]bin[5]bin[4]bin[3]bin[2]bin[1]bin[0]"00000""00000000""10";
}
else{
print opcode"000000"$2"00000""00000000""00";
}
}

op_d==2{
if(NF==3){
OFS="";
if((x=index($2,"$"))>0){
var1=substr($2,2,length($2));
binconvert(var1);
print opcode,bin[10]bin[9]bin[8]bin[7]bin[6]bin[5]bin[4]bin[3]bin[2]bin[1]bin[0]$3"0000000010";
}
else{
print opcode"000000"$2$3"0000000000";
}
}
else{
printf opcode;
if((x=index($2,"$"))>0){
var1=substr($2,2,length($2));
binconvert(var1);
printf bin[10]bin[9]bin[8]bin[7]bin[6]bin[5]bin[4]bin[3]bin[2]bin[1]bin[0];
imm=1;
}
else{
printf "000000"$2;
imm=0;
}
printf $3;
offset=substr($4,2,length($4));
binconvert(offset);
printf bin[7]bin[6]bin[5]bin[4]bin[3]bin[2]bin[1]bin[0];
print imm"1";
}
}
op_d=0;

' >> binary_file

rm out.txt
rm awkout1.txt
rm awkout2.txt
rm awkout3.txt
rm awkout4.txt

