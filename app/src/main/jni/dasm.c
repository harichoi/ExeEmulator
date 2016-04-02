#define STRICT
#define MAINPROG   

//#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <dir.h>
#include <math.h>
#include <float.h>
//#include <conio.h>
#pragma hdrstop

#include "disasm.h"
#include "dasm.h"
#include "fileheader.h"

#define GO_AHEAD 1
#define JMP 2
#define CALL 3
#define RET 4
#define ADD 5
#define MOV 6
#define SUB 7
#define LEA 8
#define POP 9
#define PUSH 10
#define INC 11
#define DEC 12
#define INT3 13
#define CMP 14
#define JE 15
#define JNE 16
#define AND 17
#define XOR 18
#define NOT 19
#define TEST 20
#define JNZ 21
#define JA 22
#define REP 23
#define SHR 24
#define JLE 25
#define JNB 26
#define JB 27
#define JBE 28
#define JAE 29
#define AND 30
#define OR 31
#define JG 32
#define JGE 33
#define JL 34
#define JNAE 35
#define JNG 36
#define JNGE 37
#define JNA 38
#define JLE 39
#define NOP 40
#define MUL 41
#define DIV 42
#define SHL 43
#define REP_STOS 44

#define TYPE_BYTE 1
#define TYPE_DWORD 4

#define DEBUG_MODE 0

IMAGE_DOS_HEADER idh;
IMAGE_FILE_HEADER ifh;
IMAGE_OPTIONAL_HEADER32 ioh;
IMAGE_SECTION_HEADER ish[10];

IMAGE_IMPORT_DESCRIPTOR iid[30];
IMAGE_IMPORT_BY_NAME iibn[10][100];
DWORD iats[10][100];
char *iatsName[10][100];
int nIid=0;
int nIibn[10];


DWORD *queueInput[100];
char inputType[100];
int rearInput=0,frontInput=0;

char printBuffer[10000];
int printPt=0;

//�Ʒ��κ��� ���߿� Flag ����(ZF,CF,SF,OF) �� ��ȯ�� ��ȹ��
#define false 0
#define true 1

bool cmpJe=false;
bool cmpJne=false;
bool cmpJa=false;
bool cmpJae=false;
bool cmpJb=false;
bool cmpJbe=false;
bool cmpJg=false;
bool cmpJge=false;
bool cmpJl=false;
bool cmpJle=false;
bool cmpJnae=false;
bool cmpJng=false;
bool cmpJnge=false;
bool cmpJna=false;
bool cmpJnb=false;

DWORD eip;
DWORD *stack;
int heapPt;
DWORD esp=-50,ebp=-50;
DWORD eax=0,ebx=0,ecx=0,edx=0,esi=0,edi=0;

long memoryAddressTable[100];
unsigned char *fileContent;

bool isFinished=false;
void printToBuffer(char *str){
	char *pt=printBuffer+printPt;
	sprintf(pt,"%s",str);
	printPt+=strlen(str);
	printBuffer[printPt]=0;
}
void printBufferClear(){
	printPt=0;
	printBuffer[0]=0;
}

char *getPrintBuffer(){
	if(printPt==0) return NULL;
	return printBuffer;
}
void inputChar(char *str){
	if(rearInput==frontInput) return;
	frontInput=(frontInput+1)%100;
	memcpy(queueInput[frontInput],str,strlen(str)+1);
}
void inputInt(int num){
	if(rearInput==frontInput) return;
	frontInput=(frontInput+1)%100;
	queueInput[frontInput][0]=num;
}
void inputFloat(float num){
	if(rearInput==frontInput) return;
	frontInput=(frontInput+1)%100;
	queueInput[frontInput][0]=num;
}
int getInputType(){
	if(frontInput==rearInput){
		return 0;
	}
	if(inputType[frontInput+1]=='s' || inputType[frontInput+1]=='S'){
		return 1;
	}else if(inputType[frontInput+1]=='f' || inputType[frontInput+1]=='d'){
		return 2;
	}
	return 3;
}

bool isFinish(){
	if(ebp<0 || esp<0) return true;
	if(eip<ioh.ImageBase) return true;

	return isFinished;
}
void fileLoad(char *fileName, long stackSize, long heapSize){
	int i;
	stack=malloc((stackSize+heapSize)/4*sizeof(DWORD));
	for(i=0 ; i<(stackSize+heapSize)/4 ; i++) stack[i]=0;
	heapPt=-(stackSize+heapSize)+50;

	FILE *fp;
	fp=fopen(fileName,"rb");
	fseek(fp,0,SEEK_END);
	long fileLength=ftell(fp);

	fileContent=malloc(fileLength * sizeof(unsigned char));
	fseek(fp,0,SEEK_SET);
	fread(fileContent,1,fileLength,fp);

	fseek(fp,0,SEEK_SET);
	fread(&idh,1,sizeof(IMAGE_DOS_HEADER),fp);

	//fseek(fp,0x40,SEEK_SET);
	//fread(&dosstub,1,sizeof(),fp);

	fseek(fp,idh.e_lfanew+4,SEEK_SET);
	if(DEBUG_MODE){
		printf("����ũ�� : %08X\n",fileLength); 
		printf("IMAGE FILE HEADER : %X\n",ftell(fp));
	}
	fread(&ifh,1,sizeof(IMAGE_FILE_HEADER),fp);

	if(DEBUG_MODE){
		printf("IMAGE OPTIOAN HEADER : %X\n",ftell(fp));
	}
	fread(&ioh,1,sizeof(IMAGE_OPTIONAL_HEADER32),fp);
	
	if(DEBUG_MODE){
		printf("Address Of Entry Point : %x\n",ioh.AddressOfEntryPoint);
		printf("Base Of Code : %x\n",ioh.BaseOfCode);
		printf("Base Of Data : %x\n",ioh.BaseOfData);
		printf("Image Base : %x\n",ioh.ImageBase);
		printf("Number Of Directories : %x\n",ioh.NumberOfRvaAndSizes);
	}
/*
	for(int i=0 ; i<ioh.NumberOfRvaAndSizes ; i++){
		printf("%x",ioh.DataDirectory[i].VirtualAddress);
//1 : import directory
	}*/


	for(i=0; i<ifh.NumberOfSections ; i++){
		int location=ftell(fp);
		fread(&ish[i],1,sizeof(IMAGE_SECTION_HEADER),fp);//section header : text
		if(DEBUG_MODE){
			printf("--------Section%d--------\n",i);
			printf("%X\n",location);
			printf("Name: %s\n",ish[i].Name);
			printf("Size Of RawData: %X\n",ish[i].SizeOfRawData);
			printf("Pointer To RawData: %X\n",ish[i].PointerToRawData);
			printf("Virtual Address: %X\n",ish[i].VirtualAddress);
		}
		memoryAddressTable[i]=ish[i].VirtualAddress+ioh.ImageBase;
		if(DEBUG_MODE){
			printf("Memory Address: %X\n",memoryAddressTable[i]);
			printf("\n");
		}
	}

	long seekPlace=getRawAddr(ioh.DataDirectory[1].VirtualAddress+ioh.ImageBase);
	if(DEBUG_MODE){
		printf("Dll import : %8x\n",getRawAddr(ioh.DataDirectory[1].VirtualAddress+ioh.ImageBase));
	}
	while(true){
		IMAGE_IMPORT_DESCRIPTOR iid_temp;
		fseek(fp,seekPlace,SEEK_SET);
		fread(&iid_temp,1,sizeof(IMAGE_IMPORT_DESCRIPTOR),fp);
		seekPlace=ftell(fp);
		if(iid_temp.FirstThunk==0x00)
			break;
		iid[nIid]=iid_temp;
		nIibn[nIid]=0;

////////////////////////////////
		char *ptr=malloc(100*sizeof(char));
		fseek(fp,getRawAddr(iid[nIid].Name+ioh.ImageBase),SEEK_SET);
		fread(ptr,1,100,fp);
		if(DEBUG_MODE){
			printf("%x(%s).....\n",getRawAddr(iid[nIid].Name+ioh.ImageBase),ptr);
		}
		fseek(fp,getRawAddr(iid[nIid].OriginalFirstThunk+ioh.ImageBase),SEEK_SET);
		
		DWORD iibnAddrs[100];
		while(true){
			DWORD iibnAddr;
			fread(&iibnAddr,1,sizeof(DWORD),fp);

			if(iibnAddr==0x00)
				break;
			iibnAddrs[nIibn[nIid]]=iibnAddr;
			nIibn[nIid]++;
		}	
		for(i=0; i<nIibn[nIid]; i++){
			fseek(fp,getRawAddr(iibnAddrs[i]+ioh.ImageBase),SEEK_SET);
			fread(&iibn[nIid][i],1,100,fp);
			iats[nIid][i]=iid[nIid].FirstThunk+4*i+ioh.ImageBase;
			if(DEBUG_MODE){
				printf("%s.%s(%8X)\n",ptr,iibn[nIid][i].Name,iats[nIid][i]);
			}
			iatsName[nIid][i]=malloc(sizeof(char)*strlen((char *)iibn[nIid][i].Name)+strlen(ptr)+5);
			sprintf(iatsName[nIid][i],"%s",iibn[nIid][i].Name);
		}
		nIid++;
	}

	eip=ioh.AddressOfEntryPoint+ioh.ImageBase;

	if(DEBUG_MODE){
		printf("nIid : %d\n",nIid);
		printf("EIP : %x\n",eip);
		//getch();
	}
		
	fclose(fp);
}

long getRawAddr(long memoryAddress){
	int i;
	for(i=1; i<ifh.NumberOfSections ; i++){
		if(memoryAddress<memoryAddressTable[i]){
			return memoryAddress-ioh.ImageBase-ish[i-1].VirtualAddress+ish[i-1].PointerToRawData;	
		}
	}
	return memoryAddress-ioh.ImageBase-ish[ifh.NumberOfSections-1].VirtualAddress+ish[ifh.NumberOfSections-1].PointerToRawData;
}
int flag=0;

void print(char *str,DWORD target){
	if(target>=0) printf("%s : %8d ",str,target);
	else printf("%s : %8x ",str,target);
}
char * readNext(){
	int i, j, k;
	if(DEBUG_MODE){

		for(i=ebp ; i<=esp ; i+=4)
			printf("STACK[%d]=%8x\n",i,stack[-i/4]);
		if(ebp!=esp){
			for(i=esp ; i<=ebp ; i+=4)
				printf("STACK[%d]=%8x\n",i,stack[-i/4]);
		}

		print("EBP",ebp);
		print("ESP",esp);
		print("ESI",esi);
		print("EDI",edi);
		printf("\n");
		print("EAX",eax);
		print("EBX",ebx);
		print("ECX",ecx);
		print("EDX",edx);
		printf("\n------------------\n");
	}
//getch();
//if(flag==1) getch();
  	ulong l;
  	t_disasm da;

  	ideal=1; lowercase=1; putdefseg=1;
  	l=Disasm((char *)(fileContent+getRawAddr(eip)),100,eip,&da,DISASM_CODE);

	struct AsmCode asmCode=getAsmCode(da.result);
	char debugStr[256];
	sprintf(debugStr,"%08X : %-25s ",eip,da.result);

	if(DEBUG_MODE){
		if(asmCode.source.data!=NULL && asmCode.destination.data !=NULL){
			printf("%08X : %-25s ",eip,da.result);
			printf("[%08X,%08X]\n",asmCode.source.data[0],asmCode.destination.data[0]);
		}else if(asmCode.source.data!=NULL && asmCode.destination.data ==NULL){
			printf("%08X : %-25s ",eip,da.result);
			printf("[S:%08X]\n",asmCode.source.data[0]);
		}else if(asmCode.source.data==NULL && asmCode.destination.data !=NULL){
			printf("%08X : %-25s ",eip,da.result);
			printf("[D:%08X]\n",asmCode.destination.data[0]);
		}else if(asmCode.source.data==NULL && asmCode.destination.data ==NULL){
			printf("%08X : %-25s\n",eip,da.result);
		}
	}
	switch(asmCode.opcode){

	case SHR:
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL)
			asmCode.source.data[0]=asmCode.source.data[0]>>asmCode.destination.data[0];
		break;
	case SHL:
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL)
			asmCode.source.data[0]=asmCode.source.data[0]<<asmCode.destination.data[0];
		break;
	case MUL: //���߿� ��, ���� DWORD�� �и����Ѿ� �� 
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL)
			asmCode.source.data[0]*=asmCode.destination.data[0];
		else if(asmCode.source.data!=NULL)
			asmCode.source.data[0]*=eax;
		break;
	case DIV:  //���߿� ��, ���� DWORD�� �и����Ѿ� �� 
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL)
			asmCode.source.data[0]/=asmCode.destination.data[0];
		else if(asmCode.source.data!=NULL)
			asmCode.source.data[0]*=eax;
		break;
	case XOR:
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL)
			asmCode.source.data[0]^=asmCode.destination.data[0];
		break;
	case NOT:
		if(asmCode.source.data!=NULL)
			asmCode.source.data[0]=~asmCode.source.data[0];
		break;
	case CMP:
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL){
			cmpJe=asmCode.source.data[0]==asmCode.destination.data[0];
			cmpJne=asmCode.source.data[0]!=asmCode.destination.data[0];
			cmpJa=asmCode.source.data[0]>asmCode.destination.data[0];
			cmpJae=asmCode.source.data[0]>=asmCode.destination.data[0];
			cmpJb=asmCode.source.data[0]<asmCode.destination.data[0];
			cmpJbe=asmCode.source.data[0]<=asmCode.destination.data[0];
			cmpJg=asmCode.source.data[0]>asmCode.destination.data[0];
			cmpJge=asmCode.source.data[0]>=asmCode.destination.data[0];
			cmpJl=asmCode.source.data[0]<asmCode.destination.data[0];
			cmpJle=asmCode.source.data[0]<=asmCode.destination.data[0];
			cmpJnae=asmCode.source.data[0]<asmCode.destination.data[0];
			cmpJng=asmCode.source.data[0]<=asmCode.destination.data[0];
			cmpJnge=asmCode.source.data[0]<asmCode.destination.data[0];
			cmpJna=asmCode.source.data[0]<=asmCode.destination.data[0];
			cmpJnb=asmCode.source.data[0]>=asmCode.destination.data[0];
		}
		break;
	case REP_STOS:
		for(i=0 ; i<ecx ; i++){
			stack[-edi/4]=eax;
			edi+=4;
		}
		break;
	case DEC:
		if(asmCode.source.data!=NULL)
			asmCode.source.data[0]--;
		break;
	case INC:
		if(asmCode.source.data!=NULL)
			asmCode.source.data[0]++;
		break;
	case AND:
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL)
			asmCode.source.data[0]&=asmCode.destination.data[0];
		break;
	case OR:
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL)
			asmCode.source.data[0]|=asmCode.destination.data[0];
		break;
	case SUB:
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL)
			asmCode.source.data[0]-=asmCode.destination.data[0];
		break;
	case ADD:
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL)
			asmCode.source.data[0]+=asmCode.destination.data[0];
		break;

	case LEA:
		if(DEBUG_MODE)
			printf("LEA : %d, %d \n",stack,asmCode.destination.data);
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL)
			asmCode.source.data[0]=(stack-asmCode.destination.data)*4;
		if(DEBUG_MODE){
			for(i=0; i<1000 ; i++)
				if(asmCode.destination.data==stack+i)
					printf("stack[%d]=%d\n",i,stack+i);
		}
		//getch();
		break;
//	case LEA:
	case MOV:
		if(asmCode.source.data!=NULL && asmCode.destination.data!=NULL)
			asmCode.source.data[0]=asmCode.destination.data[0];
		break;
	case PUSH:
		esp-=4;
		if(asmCode.source.data!=NULL)
			stack[-esp/4]=asmCode.source.data[0];
		if(DEBUG_MODE)
			printf("stack[%d] : %8x\n",-esp,stack[-esp/4]);
		break;
	case POP:
		if(asmCode.source.data)
			asmCode.source.data[0]=stack[-esp/4];
		if(DEBUG_MODE)
			printf("stack[%d] : %8x\n",-esp,stack[-esp/4]);
		esp+=4;
		break;
	case JE:
	case JNE:
	case JA:
	case JAE:
	case JB:
	case JBE:
	case JG:
	case JGE:
	case JL:
	case JLE:
	case JNAE:
	case JNG:
	case JNGE:
	case JNA:
	case JNB:
	case JMP:
	case CALL:
		if(asmCode.opcode==JE && !cmpJe)break;
		if(asmCode.opcode==JNE && !cmpJne)break;
		if(asmCode.opcode==JA && !cmpJa)break;
		if(asmCode.opcode==JAE && !cmpJae)break;
		if(asmCode.opcode==JB && !cmpJb)break;
		if(asmCode.opcode==JBE && !cmpJbe)break;
		if(asmCode.opcode==JG && !cmpJg)break;
		if(asmCode.opcode==JGE && !cmpJge)break;
		if(asmCode.opcode==JL && !cmpJl)break;
		if(asmCode.opcode==JLE && !cmpJle)break;
		if(asmCode.opcode==JNAE && !cmpJnae)break;
		if(asmCode.opcode==JNG && !cmpJng)break;
		if(asmCode.opcode==JNGE && !cmpJnge)break;
		if(asmCode.opcode==JNA && !cmpJna)break;
		if(asmCode.opcode==JNB && !cmpJnb)break;
	
		if(asmCode.opcode==CALL){
			esp-=4;
			stack[-esp/4]=(long)eip+l;
			if(DEBUG_MODE)
				printf("stack[%d] : %8x\n",-esp,stack[-esp/4]);
		}
		if(da.jmpconst!=0){
			eip=da.jmpconst;
		} else{
			DWORD addr=asmCode.source.data[0];
			eip+=l;
			for(i=0 ; i<nIid ; i++){
				for(j=0 ; j<nIibn[i] ; j++){
					if(addr==iats[i][j]){
						sprintf(debugStr,"%s , %s ",debugStr,iatsName[i][j]);
						if(DEBUG_MODE){
							printf("================\n%s Called\n=============\n",iatsName[i][j]);
						}
						char *temp=iatsName[i][j]+2;
						if(strncmp(temp,"2@YAPAXI@Z",strlen("2@YAPAXI@Z"))==0){
							if(DEBUG_MODE){
								printf("�޸��Ҵ� �������� : %d\n",-esp/4);
								for(k=1; k<10 ; k++){
									 
									if(stack[-esp/4-k]>=0)
										printf("stack[%d] : %8d\n",-esp-k*4,stack[-esp/4-k]);
									else
										printf("stack[%d] : %8x\n",-esp-k*4,stack[-esp/4-k]);
								} 
								//getch();
							} 
							eax=heapPt;
							heapPt+=stack[-esp/4-1];
							
						}
						if(strncmp(iatsName[i][j],"exit",strlen("exit"))==0){
							isFinished=true;
							return "exit";
						}
						if(strncmp(iatsName[i][j],"printf",strlen("printf"))==0){
							if(DEBUG_MODE){
								printf("PRINTF ȣ��!!\n");
								for(k=1; k<10 ; k++){
									if(stack[-esp/4-k]>=0)
										printf("stack[%d] : %8d\n",-esp-k*4,stack[-esp/4-k]);
									else
										printf("stack[%d] : %8x\n",-esp-k*4,stack[-esp/4-k]);
								} 
								printf("���� ���� : %d\n",-esp/4);
							}
							char *chkstr=(char *)(fileContent+getRawAddr(stack[-esp/4-1]));
							int chkflag=0;
							int n=1;
							while(chkstr[0]!=0x00){
								if(chkflag==1 && (chkstr[0]<'0' || chkstr[0]>'9') && chkstr[0]!=' '){			
									char tmp[1024];
									char ooo[50];
									sprintf(ooo,"%%%c",chkstr[0]);
								
									if(chkstr[0]=='s' || chkstr[0]=='S'){
										if((long)stack[-esp/4-1-n]>0){
											sprintf(tmp,ooo,(char *)fileContent+getRawAddr(stack[-esp/4-1-n]));
										} else {
											sprintf(tmp,ooo,&stack[-stack[-esp/4-1-n]/4]);
										}
									} else{
										sprintf(tmp,ooo,stack[-esp/4-1-n]);
									}
									n++;
									chkflag=0;
									printToBuffer(tmp);
								} else if(chkflag==0 && chkstr[0]!='%'){
									char temp[2];
									temp[0]=chkstr[0];
									temp[1]=0;
									printToBuffer(temp);
								}
								if(chkstr[0]=='%'){
									chkflag=1;
								}
								chkstr++;
							}
						//	printf((char *)(fileContent+getRawAddr(stack[-esp/4-1])),stack[-esp/4-2],stack[-esp/4-3],stack[-esp/4-4]);
							if(DEBUG_MODE){
								//getch();
							} 
							flag=1; 
						}
						if(strncmp(iatsName[i][j],"scanf",strlen("scanf"))==0){
							if(DEBUG_MODE){
								printf("SCANF ȣ��!!\n");
								for(k=1; k<10 ; k++){
									if(stack[-esp/4-k]>=0)
										printf("ARG %i : %8d\n",k,stack[-esp/4-k]);
									else
										printf("ARG %i : %8x\n",k,stack[-esp/4-k]);
								} 
							}
					
							if(DEBUG_MODE){
								//getch();
							} 
							char *chkstr=(char *)(fileContent+getRawAddr(stack[-esp/4-1]));
							int nParameter=0;
							char parameters[10];
							int chkflag=0;
							while(chkstr[0]!=0x00){
								if(chkflag==1 && (chkstr[0]<'0' || chkstr[0]>'9') && chkstr[0]!=' '){
									parameters[nParameter]=chkstr[0];
									nParameter++;
									chkflag=0;
								}
								if(chkstr[0]=='%'){
									chkflag=1;
								}
								chkstr++;
							}
							for(k=0 ; k<nParameter ; k++){
								DWORD *target=&stack[-stack[-esp/4-2-k]/4];
								rearInput=(rearInput+1)%100;
								inputType[rearInput]=parameters[k];
								queueInput[rearInput]=target;
							}
							//scanf((char *)(fileContent+getRawAddr(stack[-esp/4-1])),&stack[-esp/4-2],&stack[-esp/4-3],&stack[-esp/4-4],&stack[-esp/4-5]);

							if(DEBUG_MODE){
								for(k=1; k<10 ; k++){
									if(stack[-esp/4-k]>=0)
										printf("stack[%d] : %8d\n",-esp-k*4,stack[-esp/4-k]);
									else
										printf("stack[%d] : %8x\n",-esp-k*4,stack[-esp/4-k]);
								} 
							} 
							flag=1; 
						}
						eip=stack[-esp/4];
						esp+=4;	
						return debugStr;
					}
				}
			}
			
			eip=stack[-esp/4];
			esp+=4;	
		}
		return debugStr;
	case RET: 
		eip=stack[-esp/4];
		esp+=4;

		sprintf(debugStr,"%s , %08X : %-25s ",debugStr,eip,da.result);

			return debugStr;
	default:
		break;
	}
	eip+=l;
	return debugStr;
}

struct AsmSubCode getOprand(char *code){
	int i;
	struct AsmSubCode result;
	if(!strstr(code,"[")){
		if(strncmp(code,"eax",3)==0) result.data=&eax;
		else if(strncmp(code,"ebx",3)==0) result.data=&ebx;
		else if(strncmp(code,"ecx",3)==0) result.data=&ecx;
		else if(strncmp(code,"edx",3)==0) result.data=&edx;
		else if(strncmp(code,"esp",3)==0) result.data=&esp;
		else if(strncmp(code,"ebp",3)==0) result.data=&ebp;
		else if(strncmp(code,"edi",3)==0) result.data=&edi;
		else if(strncmp(code,"esi",3)==0) result.data=&esi;
		else{
			DWORD *addr=malloc(sizeof(DWORD));
			addr[0]=strtoul(code, NULL, 16);
			result.data=addr;
		}
	} else{
		code++;
		for(i=0; i<strlen(code) ; i++){
			if(code[i]==']') code[i]=0;
		}
		if(strncmp(code,"dword",5)==0){
			result.type=TYPE_DWORD;
			code+=6;
		}
		if(strncmp(code,"byte",4)==0){
			result.type=TYPE_BYTE;
			code+=5;
		}
		if(strncmp(code,"ss:",3)==0){
			code+=3;
			DWORD *ptr=NULL;
			if(strncmp(code,"eax",3)==0) ptr=&eax;
			else if(strncmp(code,"ebx",3)==0) ptr=&ebx;
			else if(strncmp(code,"ecx",3)==0) ptr=&ecx;
			else if(strncmp(code,"edx",3)==0) ptr=&edx;
			else if(strncmp(code,"esp",3)==0) ptr=&esp;
			else if(strncmp(code,"ebp",3)==0) ptr=&ebp;
			else if(strncmp(code,"edi",3)==0) ptr=&edi;
			else if(strncmp(code,"esi",3)==0) ptr=&esi;
			code+=3;
			if(code[0]=='-'){
				code++;
				long addr=((long)ptr[0]-strtoul(code, NULL, 16));
				if(addr<0){
					result.data=&stack[-addr/4];
					if(DEBUG_MODE){
						printf("stack[%d]=%d\n",-addr,stack[-addr/4]);
					}
				} else{
					DWORD *addr=malloc(sizeof(DWORD));
					addr[0]=fileContent[getRawAddr(addr)];
					result.data=addr;
				}
			} else {
				long addr=((long)ptr[0]+strtoul(code, NULL, 16));
				if(addr<0){
					result.data=&stack[-addr/4];
					if(DEBUG_MODE){
						printf("stack[%d]=%d\n",-addr,stack[-addr/4]);
					}
				} else{
					DWORD *addr=malloc(sizeof(DWORD));
					addr[0]=fileContent[getRawAddr(addr)];
					result.data=addr;
				}
			}
		} else if(strncmp(code,"ds:",3)==0){
			code+=3;
			DWORD *addr=malloc(sizeof(DWORD));
			addr[0]=strtoul(code, NULL, 16);
			result.data=addr;
		}else if(strncmp(code,"fs:",3)==0){ //�ӽ� 
			code+=3;
			DWORD *addr=malloc(sizeof(DWORD));
			addr[0]=strtoul(code, NULL, 16);
			result.data=addr;
		}
	}
	return result;
}


struct AsmCode getAsmCode(char *code){
	char *str=malloc(sizeof(char)*(strlen(code)+1));
	sprintf(str,"%s",code);
	struct AsmCode asmCode;
	asmCode.source.data=NULL;
	asmCode.destination.data=NULL;
	if(strncmp(code,"jmp",3)==0) asmCode.opcode=JMP;
	else if(strncmp(code,"rep stos",8)==0) asmCode.opcode=REP_STOS;
	else if(strncmp(code,"push",4)==0) asmCode.opcode=PUSH;
	else if(strncmp(code,"call",4)==0) asmCode.opcode=CALL;
	else if(strncmp(code,"int3",4)==0) asmCode.opcode=INT3;
	else if(strncmp(code,"jnae",4)==0) asmCode.opcode=JNAE;
	else if(strncmp(code,"test",4)==0) asmCode.opcode=TEST;
	else if(strncmp(code,"jnge",4)==0) asmCode.opcode=JNGE;
	else if(strncmp(code,"imul",4)==0) asmCode.opcode=MUL;
	else if(strncmp(code,"mov",3)==0) asmCode.opcode=MOV;
	else if(strncmp(code,"add",3)==0) asmCode.opcode=ADD;
	else if(strncmp(code,"pop",3)==0) asmCode.opcode=POP;
	else if(strncmp(code,"jne",3)==0) asmCode.opcode=JNE;
	else if(strncmp(code,"and",3)==0) asmCode.opcode=AND;
	else if(strncmp(code,"cmp",3)==0) asmCode.opcode=CMP;
	else if(strncmp(code,"sub",3)==0) asmCode.opcode=SUB;
	else if(strncmp(code,"and",3)==0) asmCode.opcode=AND;
	else if(strncmp(code,"xor",3)==0) asmCode.opcode=XOR;
	else if(strncmp(code,"lea",3)==0) asmCode.opcode=LEA;
	else if(strncmp(code,"???",3)==0) asmCode.opcode=RET;
	else if(strncmp(code,"ret",3)==0) asmCode.opcode=RET;
	else if(strncmp(code,"dec",3)==0) asmCode.opcode=DEC;
	else if(strncmp(code,"inc",3)==0) asmCode.opcode=INC;
	else if(strncmp(code,"not",3)==0) asmCode.opcode=NOT;
	else if(strncmp(code,"rep",3)==0) asmCode.opcode=REP;
	else if(strncmp(code,"jle",3)==0) asmCode.opcode=JLE;
	else if(strncmp(code,"shr",3)==0) asmCode.opcode=SHR;
	else if(strncmp(code,"shl",3)==0) asmCode.opcode=SHL;
	else if(strncmp(code,"jnb",3)==0) asmCode.opcode=JNB;
	else if(strncmp(code,"jae",3)==0) asmCode.opcode=JAE;
	else if(strncmp(code,"jbe",3)==0) asmCode.opcode=JBE;
	else if(strncmp(code,"jge",3)==0) asmCode.opcode=JGE;
	else if(strncmp(code,"jng",3)==0) asmCode.opcode=JNG;
	else if(strncmp(code,"jna",3)==0) asmCode.opcode=JNA;
	else if(strncmp(code,"jle",3)==0) asmCode.opcode=JLE;
	else if(strncmp(code,"and",3)==0) asmCode.opcode=AND;
	else if(strncmp(code,"nop",3)==0) asmCode.opcode=NOP;
	else if(strncmp(code,"jnz",3)==0) asmCode.opcode=JNE;
	else if(strncmp(code,"mul",3)==0) asmCode.opcode=MUL;
	else if(strncmp(code,"div",3)==0) asmCode.opcode=DIV;
	else if(strncmp(code,"jg",2)==0) asmCode.opcode=JG;
	else if(strncmp(code,"jb",2)==0) asmCode.opcode=JB;
	else if(strncmp(code,"je",2)==0) asmCode.opcode=JE;
	else if(strncmp(code,"jz",2)==0) asmCode.opcode=JE;
	else if(strncmp(code,"ja",2)==0) asmCode.opcode=JA;
	else if(strncmp(code,"or",2)==0) asmCode.opcode=OR;
	else if(strncmp(code,"jl",2)==0) asmCode.opcode=JL;

	else if(DEBUG_MODE){
		printf("[%s]\n",code);
		//Sleep(1000);
		return asmCode;
	}
	while(str[0]!=0 && str[0]!=' ') str++;

	if(strlen(str)==0) return asmCode;

	str++;
	
	char *source=strtok(str,",");
	char *destination=NULL;

	if(source) destination=strtok(NULL,",");

	if(source)	asmCode.source=getOprand(source);
	if(destination)	asmCode.destination=getOprand(destination);
	
	return asmCode;
}
