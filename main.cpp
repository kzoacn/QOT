#include<bits/stdc++.h>
#include "picosha2.h"
#include "bch_codec.h"
#include "bch_codec.c"
#include "qot.hpp"
#include "NetIO.hpp"
using namespace std;

int party,port;
NetIO *io;

 
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

            if(rand()%100<2){
                qb2.value^=1;
            }

        }else{
            qb2.value=rand()%2;
        }
        bob.push_back(qb2);
    }

}



vector<unsigned char> QOT(int party,NetIO *io,vector<Qubit>&qubits,int length,vector<unsigned char>message[2],bool b){

    int n=8*512;

    vector<Com>coms;
    vector<Decom>decoms;
    vector<unsigned char>chck; 
    if(party==BOB){
     
        for(int i=0;i<n;i++){
            Decom d;
            d.s[0]=qubits[i].basis;
            d.s[1]=qubits[i].value;
            Com com;
            com=Commit(d);
            coms.push_back(com);
            decoms.push_back(d);
        }
        for(int i=0;i<n;i++){
            io->send_data(coms[i].hash.data(),coms[i].hash.size());
        }
        io->flush();

    }else{
        coms.resize(n);
        for(int i=0;i<n;i++){
            coms[i].hash.resize(picosha2::k_digest_size);
            io->recv_data(coms[i].hash.data(),coms[i].hash.size());
        }

    
        for(int i=0;i<n;i++)
            chck.push_back(rand()%2);//TODO crypto random
        
        io->send_data(chck.data(),chck.size());
        io->flush();
    }

    if(party==BOB){
        chck.resize(n);
        io->recv_data(chck.data(),chck.size());

    }
 

    int all=0,yes=0;
    for(int i=0;i<n;i++){
        if(!chck[i])continue;

        Qubit qb;
        Decom decom;

        if(party==ALICE){
            io->recv_data(&decom,sizeof(decom));
            qb.basis=decom.s[0];
            qb.value=decom.s[1];
        }else{
            decom=decoms[i];
            io->send_data(&decom,sizeof(decom));
            io->flush();
        }

        if(!Open(coms[i],decom)){
            fprintf(stderr,"Commit Fail!!");
        }

        if(qubits[i].basis==qb.basis){
            all++;
            if(qubits[i].value==qb.value)
                 yes++;
        }
    }
 

    if(party==ALICE){
        cerr<<yes<<"/"<<all<<endl;
        if(yes>=all*0.95){
            fprintf(stderr,"Check Pass\n");
        }else{
            fprintf(stderr,"Check Fail\n");
            exit(-1);
        }
    }
   

    vector<Qubit>rest;
    vector<unsigned char>basis;
    for(int i=0;i<n;i++){
        if(!chck[i]){
            rest.push_back(qubits[i]);
            basis.push_back(qubits[i].basis);
        }
    }
    qubits=rest;
    n=qubits.size();

    
    //alice sending basis
    if(party==ALICE){
        io->send_data(basis.data(),basis.size());
        io->flush();
    }else{
        io->recv_data(basis.data(),basis.size());
    }
    

    vector<unsigned char>I;

    for(int i=0;i<n;i++){
        int d= basis[i]==qubits[i].basis?1:0;
        if(b==1)
            I.push_back(d);
        else
            I.push_back(d^1);
    }

    //send I[2]

    if(party==BOB){
        io->send_data(I.data(),I.size());
        io->flush();
    }else{
        io->recv_data(I.data(),I.size());
    }

    vector<unsigned char>bits[2];

    for(int i=0;i<n;i++){
        bits[I[i]].push_back(qubits[i].value);
    }
    cerr<<bits[0].size()<<endl;
    cerr<<bits[1].size()<<endl;


    const int m=9,t=55;
    bch_control * bch = init_bch(m,t,0);
    int N = (1<<m)-1;
    int msgBits = N - bch->ecc_bits;
    if(length>msgBits){
        cerr<<"NO! msg too long!"<<endl;
    }
    cerr<<"BCH parameter "<<N<<" "<<msgBits<<" "<<t<<endl;
      
    vector<unsigned char >data;
    data.resize(N);  

    if(party==ALICE){
        for(int j=0;j<2;j++){
            if(bits[j].size()<N){
                fprintf(stderr,"qubits not enough");
                exit(-1);
            }
            for(int i=0;i<msgBits;i++)
                data[i]=rand()%2; //TODO
            //vector<unsigned char> ecc(bch->ecc_bits);
            
            for(int i=0;i<length;i++){
                message[j][i]^=data[i]; //TODO Encryption
            }

            encodebits_bch(bch,&data[0],&data[msgBits]);
            for(int i=0;i<N;i++)
                data[i]^=bits[j][i];
            io->send_data(data.data(),data.size());
            io->send_data(message[j].data(),message[j].size());
            
            io->flush();
        }
    }else{
    
        for(int j=0;j<2;j++){
            if(bits[j].size()<N){
                fprintf(stderr,"qubits not enough");
                exit(-1);
            }
            io->recv_data(data.data(),N);
            for(int i=0;i<N;i++)
                data[i]^=bits[j][i];
            vector<unsigned int> errLocOut(t);
            int nerrFound = decodebits_bch(bch, &data[0], &data[msgBits], &errLocOut[0]);
            correctbits_bch(bch,&data[0],&errLocOut[0],nerrFound);
            message[j].resize(length);
            io->recv_data(message[j].data(),message[j].size()); 
            for(int i=0;i<length;i++){
                message[j][i]^=data[i]; //TODO Encryption
            }
            
        }
    }

    if(party==BOB){
        for(auto x:message[b^1])
            printf("%d ",x);
        puts("");
    }

    free_bch(bch);
    return message[b];
}



int main(int argc,char **argv){

    if(argc<3){
        puts("./main <party> <port>");
        return 0;
    }
    sscanf(argv[1],"%d",&party);
    sscanf(argv[2],"%d",&port);

    cerr<<port<<" "<<party<<endl;
    io=new NetIO(party==ALICE?NULL:"127.0.0.1",port);

    
    ifstream fin(party==ALICE ? "./rawkey/alice/key.bin" : "./rawkey/bob/key.bin",ios::in|ios::binary);



int T=10;
while(T--){
//    srand(123+T);
//    sim(5000,alice,bob);


//    vector<Qubit>qubits= party==ALICE? alice : bob;
    vector<Qubit>qubits= get_qubits(5000,fin);
//    cout<<qubits.size()<<endl;
//return 0;
    vector<unsigned char>message[2];
    for(int i=0;i<10;i++){
        message[0].push_back(1);
        message[1].push_back(i&1);
    }

    bool b=0; 

    auto vec=QOT(party,io,qubits,message[0].size(),message,b);
    
    if(party==BOB){
        for(auto x:vec)
            printf("%d ",x);
        puts("");
    }
    
}

    return 0;
}