#include"Testbench.h"

Testbench::Testbench(string config, DRAM* dram, SRAM* sram){
    this->dram = dram;
    this->sram = sram;
    ifstream fin(config);
    if(!fin.is_open()){
        cout<<"Testbench: Could'nt open "<<config<<endl;
        exit(1);
    }

    string line;
    getline(fin,line);

    while(line!="PRESIM_BEGIN"){
        getline(fin,line);
    }
    while(line!="PRESIM_END"){
        if(!line.empty()){
            parse_instruction(line, this,0);
        }
        getline(fin,line);
    }

    while(line!="POSTSIM_BEGIN"){
        getline(fin,line);
    }
    while(line!="POSTSIM_END"){
        if(!line.empty()){
            parse_instruction(line, this,1);
        }
        getline(fin,line);
    }
    fin.close();
}

void Testbench::begin(){
    for(int i=0; i<presim_queue.size(); i++){
        execute(presim_queue[i]);
    }
}

void Testbench::end(){
    for(int i=0; i<postsim_queue.size(); i++){
        execute(postsim_queue[i]);
    }
}

void Testbench::execute(instruction inst){
    string op = inst.op;
    long begin = inst.begin;
    long end = inst.end;
    string target = inst.target;
    uint32_t* iter;

    if(op=="dram"){
        iter=&(dram->mem[begin]);
        dram_get_input(iter,begin,end, target);
    }
    else if(op=="sram"){
        iter=&(sram->mem[begin]);
        sram_print_mem(iter,begin,end, target);
    }

}

int parse_instruction(string line, Testbench* tb,int flag){
    if(line=="POSTSIM_BEGIN" || line=="PRESIM_BEGIN")
         return 0;
    const char*  str = line.c_str();
    char op[20];
    char begin[20];
    char end[20];
    char target[30];
    char* temp;
    temp = op;
    int which=0;
    int index;

    int i=0;
    while(str[i]==' '){
        i++;
    }
    if(str[i]!='\0'&&str[i+1]!='\0')
        if(str[i]=='/'&&str[i+1]=='/')
            return 0;

    for(int j=0;j<4; j++){
        index=0;
        while(str[i]!=',' && str[i]!='\0'){
                temp[index]=str[i];
                index++;
                i++;
        }
        i++;
        temp[index]='\0';
        temp= (!which)? begin : (which==1)?end : target;
        which++;
    }

    long begin_h=0, end_h=0;
    int k=2;
    while(begin[k]!='\0'){
       begin_h=16*begin_h+hex_to_dec(begin[k]);
        k++;
    }
    k=2;
    while(end[k]!='\0'){
        end_h=16*end_h+hex_to_dec(end[k]);
        k++;
    }
    
    instruction instr;
    instr.op = string(op);
    instr.begin=begin_h;
    instr.end=end_h;
    instr.target=string(target);

    cout<<instr.op<<" "<<instr.begin<<" "<<instr.end<<" "<<instr.target<<endl;

    if(!flag)
        tb->presim_queue.push_back(instr);
    else if(flag==1)
        tb->postsim_queue.push_back(instr);

    return 0;
}

void Testbench::dram_get_input(uint32_t* iter, long begin, long end,  string filename){
    ifstream fin(filename);
    if(fin.is_open()){
        string line;
        int i=0;
        getline(fin, line);
        while(1){
            line = _16byte_to_4word(line);
            string temp;
            for(int j=4*i; j<4*i+4; j++){
                temp = line.substr(8*(j%4),8); 
                *iter = stoul(temp,nullptr,16);
                iter++;
                begin++;
                if(begin>end)
                    break;
            }
            if(begin>end)
                    break;
            getline(fin, line);
            i++;
        }
    }
    else{
        cout<<"Tb: sram: Couldn't open file "<<string(filename)<<endl;
    }

    fin.close();
}

void Testbench::sram_print_mem(uint32_t* iter, long begin, long end, string filename){
    ofstream fout(filename);
    if(fout.is_open()){
        fout<<hex;
        for(long i=begin; i<=end; i++){
            fout<<i<<"   "<<*iter<<endl;
            iter++;
        }
    }
    else{
        cout<<"Tb: dram: Couldn't open file "<<string(filename)<<endl;
    }

    fout.close();
}

string Testbench::_16byte_to_4word(string str){
    string temp;
    string final="";
    for(int i=0; i<=45; i+=3){
        temp = str.substr(i,2);
        final.append(temp);
    }
    return final;
}

long  hex_to_dec(char c){
    switch (c)
    {
    case 'A':
    case 'a':
        return 10;
    break;
    case 'B':
    case 'b':
        return 11;
    break;
    case 'C':
    case 'c':
        return 12;
    break;
    case 'D':
    case 'd':
        return 13;
    break;
    case 'E':
    case 'e':
        return 14;
    break;
    case 'F':
    case 'f':
        return 15;
    break;
    default:
        return long(int(c)-48);
    break;
    }
}