#include<stdio.h>
#include<string.h>

#define NameLength 10
#define LabelNumber 100
#define LineLength 200
#define LineNumber 200
#define baseAddr 0x10000

const char *ErrInf[] = {
	"Unknow error!",
	"It's not a lable, statement or note!",
	"Lack of parameter!",
	"Too many parameters!",
	"Syntax error!",
	"No this instruction!",
	"Couldn't find the label ",
	0
};
const char *insName[] = {
	"add",
	"sll",
	"bne",
	"lw",
	"addi",
	"j",
	0
};
const char *regName[] = {
	"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra", 0
};

typedef unsigned long dword;
typedef char NameString[NameLength];
typedef char LineString[LineLength];
typedef struct it{
	dword code[9];
	NameString label;
} insType;
typedef struct lt{
	NameString name;
	dword address;
} labType;

int NextStrPos(char *str, int i);
int NextString(char *str, int i, char *name);
int First(int Line, char *LinStr, int i, insType *ins, int insNo);
int TypeR(int k, int Line, char *LinStr, int i, insType *ins, int insNo);
int TypeI(int k, int Line, char *LinStr, int i, insType *ins, int insNo);
int TypeJ(int k, int Line, char *LinStr, int i, insType *ins, int insNo);
int GetReg(int Line, char *LinStr, int i, int *reg);
int GetRegB(int Line, char *LinStr, int i, int *reg);
int GetNum(int Line, char *LinStr, int i, int *num);
int GetLabel(int Line, char *LinStr, int i, char *label);
int GetPunc(int Line, char *LinStr, int i, char punc);
int CheckMore(int Line, char *LinStr, int i);
int RecOpe(char *name);
int RecReg(char *name);
int RegLab(char *lab, labType *label, int labNo);
void PrtErr(int Line, int k);

int ErrNum;

int main(){
	int i,j;
	int Line, labAddr;
	dword addr, insNo, labNo, code;
	LineString LinStr;
	NameString name;
	insType ins[LineNumber];
	labType label[LabelNumber];
	FILE *fp;

	Line = 0;
	ErrNum = 0;
	addr = baseAddr;
	insNo = labNo = 0;
	while(gets(LinStr)){
		Line++;
		i = 0;
		i = NextStrPos(LinStr, i);
		if(LinStr[i] == 0 || LinStr[i] == '#') continue;

		j = NextString(LinStr, i, name);
		j = NextStrPos(LinStr, j);
		if(LinStr[j] == ':'){
			strcpy(label[labNo].name, name);
			label[labNo].address = addr;
			labNo++;
			insNo = First(Line, LinStr, j+1, ins, insNo);
		}
		else{
			insNo = First(Line, LinStr, i, ins, insNo);
		}

		addr = baseAddr + (insNo<<2);
		//printf("%X\t",addr);
	}
	
	for(i = 0; i < insNo; i++){
		if(ins[i].code[1]){
			labAddr = RegLab(ins[i].label, label, labNo);
			if(labAddr < 0){
				printf("Error: %s%s!\n",ErrInf[6],ins[i].label);
				ErrNum++;
			}
			if(ins[i].code[2] < 4){
				ins[i].code[3] = labAddr >> 2;
			}
			else{
				ins[i].code[5] = ((labAddr - ins[i].code[0]) >> 2) - 1;
			}
		}
		/*printf("%X:\t",ins[i].code[0]);
		for(j = 2; j < 9; j++)
			printf("%X\t",ins[i].code[j]);
		if(ins[i].code[1])printf("%s\n",ins[i].label);
		else printf("\n");*/
	}

	if(ErrNum > 0)
		printf("There are %d errors!\n",ErrNum);
	else{
		fp = fopen("code.txt","wb");
		for(i = 0; i < insNo; i++){
			if(ins[i].code[8] < 2)
				code = (ins[i].code[2] << 26) + (ins[i].code[3] << 21) + (ins[i].code[4] << 16)
					+ (ins[i].code[5] << 11) + (ins[i].code[6] << 6) + (ins[i].code[7]);
			else if(ins[i].code[8] < 5)
				code = (ins[i].code[2] << 26) + (ins[i].code[3] << 21) + (ins[i].code[4] << 16) + ins[i].code[5];
			else
				code = (ins[i].code[2] << 26) + (ins[i].code[3]);
			for(j = 3; j >= 0; j--)
				fwrite((char *)&code + j, 1, 1, fp);
			//printf("%X\n",code);
		}
		fclose(fp);
	}

	return 0;
}

int NextStrPos(char *str, int i){
	while(str[i] != 0 &&(str[i] == ' ' || str[i] == '\t'))
			i++;
	return i;
}

int NextString(char *str, int i, char *name){
	int j;
	
	j = 0;
	while(str[i] != 0 && str[i] != ' ' && str[i] != '\t' && str[i] != ',' 
		&& str[i] != '#' && str[i] != ':' && str[i] != '(' && str[i] != ')'){
			name[j] = str[i];
			i++;
			j++;
	}
	name[j] = 0;
	//printf("%s\t",name);
	return i;
}

int First(int Line, char *LinStr, int i, insType *ins, int insNo){
	NameString name;
	int k;

	i = NextStrPos(LinStr, i);
	if(LinStr[i] == 0)
		return insNo;
	i = NextString(LinStr, i, name);
	k = RecOpe(name);
	ins[insNo].code[8] = k;			// record the op No.
	//printf("%d:\t",k);
	if(k < 0){
		PrtErr(Line, 5);
		return insNo;
	}
	if(k < 2)
		return TypeR(k, Line, LinStr, i, ins, insNo);
	if(k <5)
		return TypeI(k, Line, LinStr, i, ins, insNo);
	return TypeJ(k, Line, LinStr, i, ins, insNo);
}

int TypeR(int k, int Line, char *LinStr, int i, insType *ins, int insNo){
	NameString name;
	dword opcode, rs, rt, rd, shamt, funct;
	int reg, num;

	opcode = rs = rt = rd = shamt = funct = 0;
	// Get rd
	i = NextStrPos(LinStr, i);
	if(LinStr[i] == 0){
		PrtErr(Line, 2);
		return insNo;
	}
	i = NextString(LinStr, i, name);
	reg = RecReg(name);
	if(reg < 0)
		PrtErr(Line, 4);
	rd = reg;

	// Get rs or rt
	i = GetReg(Line, LinStr, i, &reg);
	if(i < 0)
		return insNo;
	switch(k){
		case 0:
			rs = reg;

			// Get rt
			i = GetReg(Line, LinStr, i, &reg);
			if(i < 0)
				return insNo;
			rt = reg;

			funct = 0x20;
			break;
		case 1:
			rt = reg;

			//Get shamt
			i = GetNum(Line, LinStr, i, &num);
			if(i < 0)
				return insNo;
			shamt = num;

			funct = 0;
			break;
	}

	i = CheckMore(Line, LinStr, i);
	if(i < 0)
		return insNo;

	ins[insNo].code[0] = baseAddr + (insNo<<2);
	ins[insNo].code[1] = ins[insNo].code[2] = 0;
	ins[insNo].code[3] = rs;
	ins[insNo].code[4] = rt;
	ins[insNo].code[5] = rd;
	ins[insNo].code[6] = shamt;
	ins[insNo].code[7] = funct;

	return insNo + 1;
}

int TypeI(int k, int Line, char *LinStr, int i, insType *ins, int insNo){
	NameString name;
	dword opcode, rs, rt, immediate, labF;
	int reg, num;

	opcode = rs = rt = immediate = labF = 0;
	// Get rt or rs
	i = NextStrPos(LinStr, i);
	if(LinStr[i] == 0){
		PrtErr(Line, 2);
		return insNo;
	}
	i = NextString(LinStr, i, name);
	reg = RecReg(name);
	if(reg < 0)
		PrtErr(Line, 4);
	switch(k){
		case 2:
			rs = reg;

			// Get rt
			i = GetReg(Line, LinStr, i, &reg);
			if(i < 0)
				return insNo;
			rt = reg;

			// Get label
			i = GetLabel(Line, LinStr, i, ins[insNo].label);
			if(i < 0)
				return insNo;
			labF = 1;
			
			opcode = 5;
			break;
		case 3:
			rt = reg;

			// Get offset
			i = GetNum(Line, LinStr, i, &num);
			if(i < 0)
				return insNo;
			immediate = num;

			//Get rs
			i = GetRegB(Line, LinStr, i, &reg);
			if(i < 0)
				return insNo;
			rs = reg;

			opcode = 0x23;
			break;
		case 4:
			rt = reg;

			// Get rs
			i = GetReg(Line, LinStr, i, &reg);
			if(i < 0)
				return insNo;
			rs = reg;

			// Get signExtImm
			i = GetNum(Line, LinStr, i, &num);
			if(i < 0)
				return insNo;
			immediate = num;

			opcode = 0x8;
			break;
	}

	i = CheckMore(Line, LinStr, i);
	if(i < 0)
		return insNo;

	ins[insNo].code[0] = baseAddr + (insNo<<2);
	ins[insNo].code[1] = labF;
	ins[insNo].code[2] = opcode;
	ins[insNo].code[3] = rs;
	ins[insNo].code[4] = rt;
	ins[insNo].code[5] = immediate;

	return insNo + 1;
}

int TypeJ(int k, int Line, char *LinStr, int i, insType *ins, int insNo){
	dword opcode, labF;
	
	opcode = labF = 0;
	// Get label
	i = NextStrPos(LinStr, i);
	if(LinStr[i] == 0){
		PrtErr(Line, 2);
		return insNo;
	}
	i = NextString(LinStr, i, ins[insNo].label);
	labF = 1;

	i = CheckMore(Line, LinStr, i);
	if(i < 0)
		return insNo;
	
	switch(k){
		case 5:
			opcode = 0x2;
			break;
	}

	ins[insNo].code[0] = baseAddr + (insNo<<2);
	ins[insNo].code[1] = labF;
	ins[insNo].code[2] = opcode;

	return insNo + 1;
}

int GetReg(int Line, char *LinStr, int i, int *reg){
	NameString name;

	i = GetPunc(Line, LinStr, i, ',');
	if(i < 0)
		return -1;

	i = NextString(LinStr, i, name);

	*reg = RecReg(name);
	if(*reg < 0){
		PrtErr(Line, 4);
		return -1;
	}
	return i;
}

int GetRegB(int Line, char *LinStr, int i, int *reg){
	NameString name;

	i = GetPunc(Line, LinStr, i, '(');
	if(i < 0)
		return -1;

	i = NextString(LinStr, i, name);

	*reg = RecReg(name);
	if(*reg < 0){
		PrtErr(Line, 4);
		return -1;
	}

	i = GetPunc(Line, LinStr, i, ')');
	if(i < 0)
		return -1;

	return i;
}

int GetNum(int Line, char *LinStr, int i, int *num){
	NameString name;

	i = GetPunc(Line, LinStr, i, ',');
	if(i < 0)
		return -1;

	i = NextString(LinStr, i, name);
	sscanf(name,"%d",num);

	return i;
}

int GetLabel(int Line, char *LinStr, int i, char *label){
	i = GetPunc(Line, LinStr, i, ',');
	if(i < 0)
		return -1;

	i = NextString(LinStr, i, label);

	return i;
}

int GetPunc(int Line, char *LinStr, int i, char punc){
	i = NextStrPos(LinStr, i);
	if(LinStr[i] == 0){
		PrtErr(Line, 2);
		return -1;
	}
	else if(LinStr[i] != punc){
		PrtErr(Line, 4);
		return -1;
	}
	i = NextStrPos(LinStr, i + 1);
	if(LinStr[i] == 0){
		PrtErr(Line, 2);
		return -1;
	}
	return i;
}

int CheckMore(int Line, char *LinStr, int i){
	i = NextStrPos(LinStr, i);
	if(LinStr[i] == 0 || LinStr[i] == '#')
		return i;

	PrtErr(Line, 3);
	return -1;
}

int RecOpe(char *name){
	int i;

	for(i = 0; i < 6; i++)
		if(strcmp(name, insName[i]) == 0)
			return i;

	return -1;
}

int RecReg(char *name){
	int i;

	if(name[0] != '$')
		return -1;

	for(i = 0; i < 32; i++)
		if(strcmp(name + 1, regName[i]) == 0)
			return i;

	return -1;
}

int RegLab(char *lab, labType *label, int labNo){
	int i;
	
	for(i = 0; i < labNo; i++)
		if(strcmp(lab, label[i].name) == 0)
			return label[i].address;

	return -1;
}

void PrtErr(int Line, int k){
	printf("Error %d: (Line %d)%s\n",k,Line,ErrInf[k]);
	ErrNum++;
}