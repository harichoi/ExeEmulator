typedef unsigned long DWORD;

struct AsmSubCode;
struct AsmCode;
struct AsmSubCode{
	short type;
	short offset;
	DWORD *data;
};
struct AsmCode{
	long opcode;
	struct AsmSubCode source;
	struct AsmSubCode destination;
};

struct AsmSubCode getOprand(char *code);
struct AsmCode getAsmCode(char *code);
void printToBuffer(char *str);
void printBufferClear();
void fileLoad(char *fileName, long stackSize, long heapSize);
char *readNext();
long getRawAddr(long memoryAddress);
#define bool int
bool isFinish();

void inputChar(char *str);
void inputFloat(float num);
void inputInt(int num);
int getInputType();
char *getPrintBuffer();
