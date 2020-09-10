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


int main(){
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