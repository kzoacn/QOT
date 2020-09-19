#include <vector>
#include <string>
#include "picosha2.h"
#include "bch_codec.h"
#include <iostream>

using namespace std;


const int ALICE=1;
const int BOB=2;

struct Qubit{
    int id;
    bool basis;
    bool value;
};


struct Com{
    vector<unsigned char>hash;
};
struct Decom{
    char s[2];
    char r[picosha2::k_digest_size];
};

Com Commit(Decom &de){
    vector<char> info;
    info.push_back(de.s[0]);
    info.push_back(de.s[1]);
    for(int i=0;i<picosha2::k_digest_size;i++){
        de.r[i]=rand(); //TODO: crypto random 
        info.push_back(de.r[i]);
    }

    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(info.begin(), info.end(), hash.begin(), hash.end());
    Com com;
    com.hash=hash;
    return com;
}
bool Open(Com com,Decom de){
    vector<char> info;
    info.push_back(de.s[0]);
    info.push_back(de.s[1]);
    for(int i=0;i<picosha2::k_digest_size;i++){ 
        info.push_back(de.r[i]);
    }

    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(info.begin(), info.end(), hash.begin(), hash.end());
    return com.hash==hash;
}


char to_hex_ch(unsigned char hex_val)
{
  if (hex_val >= 0 && hex_val <= 9)
    return char(hex_val + '0');
  else
    return char(hex_val - 10) + 'A';
}

std::string dec_to_hex(unsigned int dec)
{
  std::string hex = "";
  if (dec == 0)
    hex = "0";

  while (dec != 0)
  {
    unsigned int hex_val = dec % 16;
    hex = to_hex_ch(hex_val) + hex;
    dec /= 16;
  }
  return hex;
}
 


vector<Qubit> read_frame(ifstream &fin){
    int cnt=0;
    unsigned char tmp[4096];
    fin.read((char*)tmp,8);
   
    if(tmp[0]==0xA0 && tmp[1]==0xA1 && tmp[2]==0xA2 && tmp[3]==0xA3){
        
    }else{
        
    }
 
    int length=(1*tmp[4]<<8) | tmp[5];
    length=(length-3)/2;
    fin.read((char*)tmp,4); 
    vector<Qubit>qubits;
    if(length>100){
        return qubits;
    }

    for(int i=0;i<length;i++){
        fin.read((char*)tmp,4);
        Qubit qb;
        qb.basis=tmp[3]/2%2;
        qb.value=tmp[3]%2;
        qubits.push_back(qb);
    }
    fin.read((char*)tmp,6);
    return qubits;
}
vector<Qubit> get_qubits(int num,ifstream &fin){
    vector<Qubit>res,tmp;
    while(res.size()<num){
        tmp=read_frame(fin);
        for(auto x:tmp)
            res.push_back(x);
    }

    return res;
}

void check(){

    ifstream ain("./rawkey/alice/key.bin",ios::in|ios::binary);
    ifstream bin("./rawkey/bob/key.bin",ios::in|ios::binary);


    int cnt=0;

    int cnt_same_basis=0;
    int cnt_diff_basis=0;

    int cnt_same_same=0;
    int cnt_diff_same=0;

    for(int j=1;j<=100000;j++){
        cout<<j<<endl;
        puts("A---------");
        auto vec1=read_frame(ain);//1318325
        puts("B---------"); 
        auto vec2=read_frame(bin);

        assert(vec1.size()==vec2.size());
        for(int i=0;i<vec1.size();i++){
            Qubit qb1=vec1[i];
            Qubit qb2=vec2[i];
            if(qb1.basis==qb2.basis){
                cnt_same_basis++;
                if(qb1.value==qb2.value)
                    cnt_same_same++;
            }else{
                cnt_diff_basis++;
                if(qb1.value==qb2.value)
                    cnt_diff_same++;    
            }
        }
        printf("cnt same basis:%d\n",cnt_same_basis);
        printf("cnt diff basis:%d\n",cnt_diff_basis);

        printf("cnt same basis same value:%d/%d\n",cnt_same_same,cnt_same_basis);
        printf("cnt diff basis same value:%d/%d\n",cnt_diff_same,cnt_diff_basis);

    }
    
}
