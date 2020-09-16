#include<bits/stdc++.h>
#include "picosha2.h"
#include "bch_codec.h"
using namespace std;



struct Qubit{
    int id;
    bool basis;
    bool value;
};

vector<Qubit>alice,bob;



void sim(int n,vector<Qubit>&alice,vector<Qubit>&bob){
    alice.clear();
    bob.clear();

    for(int i=1;i<=n;i++){
        Qubit qb;
        qb.basis=rand()%2;
        qb.value=rand()%2;
        alice.push_back(qb);
        //sending

        Qubit qb2;
        qb2.basis=rand()%2;
        if(qb.basis==qb2.basis){
            qb2.value=qb.value;

            //adding noisy

            //if(rand()%100<2){
            //    qb2.value^=1;
            //}

        }else{
            qb2.value=rand()%2;
        }
        bob.push_back(qb2);
    }

}


struct Com{
    vector<unsigned char>hash;
};
struct Decom{
    string s;
    string r;
};

Com Commit(Decom &de){
    for(int i=0;i<128;i++)
        de.r+=rand()%2; //TODO: crypto random 
    string info=de.s+de.r;
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(info.begin(), info.end(), hash.begin(), hash.end());
    Com com;
    com.hash=hash;
    return com;
}
bool Open(Com com,Decom de){
    string info=de.s+de.r;
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(info.begin(), info.end(), hash.begin(), hash.end());
    return com.hash==hash;
}


void QOT(int party,vector<Qubit>&alice,vector<Qubit>&bob,string message[2],bool b,string &ret){
    int n=min(alice.size(),bob.size());
    vector<Com>coms;
    vector<Decom>decoms;

    for(int i=0;i<n;i++){
        Decom d;
        d.s+=(char)bob[i].basis;
        d.s+=(char)bob[i].value;
        Com com;
        com=Commit(d);
        coms.push_back(com);
        decoms.push_back(d);
    }

    
    vector<bool>chck;
    for(int i=0;i<n;i++)
        chck.push_back(rand()%2);
    
    int all=0,yes=0;
    for(int i=0;i<n;i++){
        if(!chck[i])continue;
        if(alice[i].basis==bob[i].basis){
            all++;
            if(!Open(coms[i],decoms[i])){
                fprintf(stderr,"Commit Fail!!");
            }
            if(alice[i].value==bob[i].value)
                yes++;
        }
    }

    cerr<<yes<<"/"<<all<<endl;
    if(yes>=all*0.99){
        fprintf(stderr,"Check Pass\n");
    }else{
        fprintf(stderr,"Check Fail\n");
    }

    vector<Qubit>rest_alice;
    vector<Qubit>rest_bob;


    vector<bool>basis;
    vector<int>I[2];

    for(int i=0;i<n;i++){
        if(!chck[i]){
            rest_alice.push_back(alice[i]);
            rest_bob.push_back(bob[i]);

            basis.push_back(alice[i].basis);
        }
    }
    int m=rest_alice.size();
    //alice sending basis

    for(int i=0;i<m;i++){
        int d= basis[i]==rest_bob[i].basis?1:0;
        I[d].push_back(i);
    }

    cerr<<I[0].size()<<" "<<I[1].size()<<endl;


    for(int i=0;i<10;i++){
        if(rest_alice[I[1][i]].value==rest_bob[I[1][i]].value){

        }else{
            cerr<<"NO!!"<<endl;
        }
    }

    if(!b){
        swap(I[0],I[1]);
    }

    //send I[2]
    for(int i=0;i<2;i++){
        if(message[i].length()>I[i].size()){
            fprintf(stderr,"length is not enough\n");
        }
        for(int j=0;j<message[i].length();j++){
            message[i][j]^=rest_alice[I[i][j]].value;
        }
    }

    //send message'


    for(int i=0;i<2;i++){
        for(int j=0;j<message[i].length();j++){
            message[i][j]^=rest_bob[I[i][j]].value;
        }
    }


    cout<<message[0]<<endl;
    cout<<message[1]<<endl;

    ret=message[b];
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

bool out=false;
int all=0;


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
    all++;
    if(length>100){
        exit(0);
    }
    int h=0;
    vector<Qubit>qubits;
    for(int i=0;i<length;i++){
        fin.read((char*)tmp,4);
        h=h*233+tmp[1];
        //if(out)
        ///    cout<<cnt<<" "<<dec_to_hex(tmp[0])<<" "<<dec_to_hex(tmp[1])<<" "<<dec_to_hex(tmp[2])<<" "<<dec_to_hex(tmp[3])<<endl;
        Qubit qb;
        qb.basis=tmp[3]/2%2;
        qb.value=tmp[3]%2;
        qubits.push_back(qb);
    }
    fin.read((char*)tmp,6);
    return qubits;
}
void parse(string file){
    ifstream fin(file,ios::in|ios::binary);
    
}

int main(){


    
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
    return 0;


    srand(time(0));
    string m[2];
    m[0]="1111111111111";
    m[1]="0000000000000";

    bool b=0;

    sim(200,alice,bob);

    string ret;
    QOT(0,alice,bob,m,b,ret);
    cout<<ret<<endl;

    return 0;
}