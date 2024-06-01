#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<string.h>
#define ORDER 3
typedef enum{Failure, Success} status_code;
typedef enum{NO, YES} lpg_stat;
typedef enum{False, True} bool;
typedef int keytype;
typedef struct Bank_info{
    char BankName[100];
    int Acc_no;
    float deposited;
}Bank_info;

typedef struct BankData{
    char Name[100];
    char Address[200];
    Bank_info Bank_details;
    int Pan_no;
}Bank_Data;
//typedef Bank_Data leafdata;
typedef struct Bankleaf{
    Bank_Data Bank_vals[ORDER+1];
    int num_vals;
    struct Bankleaf *next;
    struct Bankleaf *prev;
} Bankleaf;
//typedef Aadharleaf * leaftype;

typedef struct Internal_Bank{
    struct bplusBank *children[ORDER+1];
    keytype keys[ORDER];
    int keysize; //current number of keys
}Internal_Bank;

typedef struct bplusBank{
    bool Isleaf;
    union{
        Internal_Bank InternalNode;
        Bankleaf *LeafNode;
    } Node;
    
}Bplus_Bank;

Bankleaf *CreateLeafBank(){
    Bankleaf *new = (Bankleaf *)malloc(sizeof(Bankleaf));
    new->next=NULL;
    new->prev=NULL;
    new->num_vals = 0;
    return new;
}

Bplus_Bank *CreateBplus_Bank(bool leaf){
    Bplus_Bank *new = (Bplus_Bank *)malloc(sizeof(Bplus_Bank));
    new->Isleaf=leaf;
    if(leaf){
        new->Node.LeafNode=CreateLeafBank();
    } 
    else{
        new->Node.InternalNode.keysize=1;
        new->Node.InternalNode.children[0] = CreateBplus_Bank(True);
        new->Node.InternalNode.children[1] = CreateBplus_Bank(True);
        new->Node.InternalNode.children[0]->Node.LeafNode->prev=NULL;
        new->Node.InternalNode.children[1]->Node.LeafNode->prev = new->Node.InternalNode.children[0]->Node.LeafNode;
        new->Node.InternalNode.children[0]->Node.LeafNode->next = new->Node.InternalNode.children[1]->Node.LeafNode;
        new->Node.InternalNode.children[1]->Node.LeafNode->prev = new->Node.InternalNode.children[0]->Node.LeafNode;
        new->Node.InternalNode.children[1]->Node.LeafNode->next = NULL;
        
        
    }
    return new;

}

//STACK IMPLEMENTATION
//typedef Bplus_Bank * item_type;

typedef struct BankNode{
    Bplus_Bank *data;
    struct BankNode* next;
}BankNode;


typedef struct BankStack{
    BankNode* Top;
}BankStack;

BankNode* CreateBankNode(Bplus_Bank *d){
    BankNode *ptr;
    ptr = (BankNode *)malloc(sizeof(BankNode));
    ptr->data=d;
    ptr->next=NULL;
    return ptr;
}

void InitializeBankStack(BankStack *stptr){
    stptr->Top=NULL;
}

bool IsBankStackEmpty(BankStack s){
    return(s.Top==NULL);
}

status_code PushBank(BankStack *stptr, Bplus_Bank *d){
    status_code sc=Success;
    BankNode *ptr;
    
    ptr = CreateBankNode(d);
    //printf("Push\n");
    if(ptr == NULL) sc=Failure;
    else{
        ptr->next = stptr->Top;
        stptr->Top=ptr;
    }
    return sc;
}

status_code PopBank(BankStack *stptr, Bplus_Bank** dptr){
    status_code sc= Success;
    if(IsBankStackEmpty(*stptr)) sc=Failure;
    else{
        BankNode *ptr;
        ptr = stptr->Top;
        *dptr = ptr->data;
        stptr->Top = ptr->next;
        free(ptr);
    }
    return sc;
}

Bplus_Bank *FindBankKeyNode(Bplus_Bank *root, int key){
    int nkey = key;
    Bplus_Bank *tptr = root;
    if(tptr==NULL) return NULL;
    while(tptr->Isleaf!=True){
        int no_children = tptr->Node.InternalNode.keysize+1;
        int i=0;
        int *keys = tptr->Node.InternalNode.keys;
        no_children--;
        while(no_children--){
            if(nkey>=keys[i]){
                i++;
            }
            else no_children=0;
        }
        tptr = tptr->Node.InternalNode.children[i];      
    }
    return tptr;
}

Bplus_Bank *InsertInternalBplusBank(BankStack *pstack, keytype key, Bplus_Bank *ptr, Bplus_Bank *root){
    if(IsBankStackEmpty(*pstack)){
        //New root
        //printf("Root splitting\n");
        Bplus_Bank *new = CreateBplus_Bank(False);
        new->Node.InternalNode.keysize=1;
        //Linking not done

        new->Node.InternalNode.keys[0]=key;
        new->Node.InternalNode.children[0] = root;
        new->Node.InternalNode.children[1]= ptr;
        return new;
    }
    else{
        Bplus_Bank *parent;
        status_code sc = PopBank(pstack, &parent);
        if(!sc) printf("FAILURE");
        parent->Node.InternalNode.keysize++;
        int size = parent->Node.InternalNode.keysize;
        keytype arr[size];
        int j=0;
        int childindex;
        int inserted=0;
        for(int i=0;i<size;i++){
            if(!inserted && (j== size-1 || key<parent->Node.InternalNode.keys[j])){
                arr[i] = key;
                childindex= i+1;
                inserted++;
            }
            else{
                arr[i] = parent->Node.InternalNode.keys[j];
                j++;
            }
        }
        for(int i=0;i<size;i++){
            parent->Node.InternalNode.keys[i] = arr[i];
        }
        //Now updating children array
        //printf("childindex: %d, size: %d\n",childindex, size);
        for(int i=size;i>childindex;i--){
            parent->Node.InternalNode.children[i] = parent->Node.InternalNode.children[i-1];
        }
        parent->Node.InternalNode.children[childindex] = ptr;
        
        if(parent->Node.InternalNode.keysize>=ORDER){
            //printf("Splitting internal\n");
            //Split
            Bplus_Bank *newnode = CreateBplus_Bank(False);
            newnode->Node.InternalNode.keysize = (ORDER-1)/2;
            parent->Node.InternalNode.keysize = (ORDER-1)/2;
            for(int i=0;i<(ORDER-1)/2;i++){
                newnode->Node.InternalNode.keys[i] = parent->Node.InternalNode.keys[i+(ORDER+1)/2];
                newnode->Node.InternalNode.children[i] = parent->Node.InternalNode.children[i+(ORDER+1)/2];
                //printf("added key: %d\n",parent->Node.InternalNode.keys[i+(ORDER+1)/2]);
            }
            newnode->Node.InternalNode.children[(ORDER-1)/2] = parent->Node.InternalNode.children[ORDER];
            int key = parent->Node.InternalNode.keys[(ORDER-1)/2];
            //InsertInternal
            return InsertInternalBplusBank(pstack, key, newnode, root);
        }
        else{
            //return root by popping
            Bplus_Bank *root=parent;
            //printf("no split\n");
            while(!IsBankStackEmpty(*pstack)){
                PopBank(pstack, &root);
                //printf("popped %d\n",root->Node.InternalNode.keys[0]);
            }
            //printf("root: %d\n",root->Node.InternalNode.keys[0]);
            return root;
        }
    }
}

Bplus_Bank *InsertLeafBplusBank(Bplus_Bank *root, Bank_Data new_d){
    //Check for root null condition
    if(root==NULL){
        //printf("Inserting at root\n");
        root = CreateBplus_Bank(False);
        root->Node.InternalNode.keysize=1;
        root->Node.InternalNode.keys[0]= new_d.Bank_details.Acc_no;
        root->Node.InternalNode.children[0]->Node.LeafNode->num_vals=0;
        root->Node.InternalNode.children[1]->Node.LeafNode->Bank_vals[0]= new_d;
        root->Node.InternalNode.children[1]->Node.LeafNode->num_vals=1;
        return root;

    }
    int nkey = new_d.Bank_details.Acc_no;
    BankStack ParentStack;
    Bplus_Bank *tptr = root;
    InitializeBankStack(&ParentStack);
    while(tptr->Isleaf!=True){
        //printf("total keys %d\n", tptr->Node.InternalNode.keysize);
        PushBank(&ParentStack, tptr);
        
        int no_children = tptr->Node.InternalNode.keysize+1;
        int i=0;
        int *keys = tptr->Node.InternalNode.keys;
        no_children--;
        while(no_children--){
            if(nkey>=keys[i]){
                i++;
            }
            else no_children=0;
        }
        //printf("Chosen %d\n",i);
        tptr = tptr->Node.InternalNode.children[i];      
    }
    int numval = tptr->Node.LeafNode->num_vals;
    //printf("numval %d\n",numval);
    Bank_Data *bank_arr = tptr->Node.LeafNode->Bank_vals;
    int j=0;
    //printf("%d is key\n",nkey);
    Bank_Data arr[numval+1];
    int inserted=0;
    for(int i=0;i<numval+1;i++){
        if(!inserted && (j==numval || nkey < tptr->Node.LeafNode->Bank_vals[j].Bank_details.Acc_no)){
            arr[i] = new_d;
            inserted++;
        }
        else{
            arr[i] = tptr->Node.LeafNode->Bank_vals[j];
            j++;
        }
    }
    for(int i=0;i<numval+1;i++){
        tptr->Node.LeafNode->Bank_vals[i] = arr[i];
    }
    
    tptr->Node.LeafNode->num_vals = numval+1;
    for(int i=0;i<numval+1;i++){
        tptr->Node.LeafNode->Bank_vals[i] = arr[i];
    }
    numval++;

    if(numval==ORDER+1){
        //printf("External splitting\n");
        //SPLITTING REQUIRED
        Bplus_Bank *new = CreateBplus_Bank(True);
        new->Node.LeafNode->num_vals=(ORDER+1)/2;
        tptr->Node.LeafNode->num_vals = (ORDER+1)/2;
        int split = (ORDER+1)/2;
        for(int i=0;i<split;i++){
            new->Node.LeafNode->Bank_vals[i] = tptr->Node.LeafNode->Bank_vals[i+split];
        }
        int splitkey = new->Node.LeafNode->Bank_vals[0].Bank_details.Acc_no;
        root = InsertInternalBplusBank(&ParentStack,splitkey, new, root);
        
        //Updating the dll
        if(tptr->Node.LeafNode!=NULL){
            Bankleaf *next= tptr->Node.LeafNode->next;
            if(next==NULL){
                tptr->Node.LeafNode->next = new->Node.LeafNode;
                new->Node.LeafNode->prev = tptr->Node.LeafNode;
            }
            else{

                tptr->Node.LeafNode->next = new->Node.LeafNode;
                new->Node.LeafNode->prev = tptr->Node.LeafNode;
                new->Node.LeafNode->next = next;
                next->prev = new->Node.LeafNode;
            }
        }



    }
    return root;
    
}

void PrintBplusBankTree(Bplus_Bank *root) {
    if (root == NULL) {
        printf("Empty tree\n");
        return;
    }

    Bplus_Bank *queue[1000];
    int front = 0, rear = 0;
    queue[rear++] = root;

    while (front < rear) {
        int currentLevelSize = rear - front;
        for (int i = 0; i < currentLevelSize; i++) {
            //printf("i: %d",i);
            Bplus_Bank *currentNode = queue[front++];
            //printf("Hello\n");
            if (currentNode->Isleaf == True) {
                Bankleaf *leaf = currentNode->Node.LeafNode;
                printf("[ ");
                printf("Leafsize: %d|| ",leaf->num_vals);
                for (int j = 0; j < leaf->num_vals; j++) {
                    printf("%d:%s ", leaf->Bank_vals[j].Bank_details.Acc_no, leaf->Bank_vals[j].Name);
                }
                printf("] ");
            } 
            else {
                printf("[ ");
                for (int j = 0; j < currentNode->Node.InternalNode.keysize; j++) {
                    printf("%d ", currentNode->Node.InternalNode.keys[j]);
                }
                printf("] ");

                for (int j = 0; j <= currentNode->Node.InternalNode.keysize; j++) {
                    queue[rear++] = currentNode->Node.InternalNode.children[j];
                }
            }
        }
        printf("\n");
    }
}



//////////////////////////////////////PAN DEFINITIONS//////////////////////////////////////////////////////


typedef struct PANData{
    char Name[100];
    char Address[200];
    int PAN_no;
    int Aadhar_num;
    Bplus_Bank *Bank_DB;

    
}PAN_Data;
//typedef PAN_Data leafdata;
typedef struct PANleaf{
    PAN_Data PAN_vals[ORDER+1];
    int num_vals;
    struct PANleaf *next;
    struct PANleaf *prev;
} PANleaf;
//typedef Aadharleaf * leaftype;

typedef struct Internal_PAN{
    struct Bplus_PAN *children[ORDER+1];
    keytype keys[ORDER];
    int keysize; //current number of keys
}Internal_PAN;

typedef struct Bplus_PAN{
    bool Isleaf;
    union{
        Internal_PAN InternalNode;
        PANleaf *LeafNode;
    } Node;
    
}Bplus_PAN;

PANleaf *CreateLeafPAN(){
    PANleaf *new = (PANleaf *)malloc(sizeof(PANleaf));
    new->next=NULL;
    new->prev=NULL;
    new->num_vals = 0;
    for(int i=0;i<ORDER;i++){
        new->PAN_vals[i].Bank_DB=NULL;
    }
    //BankDB null??????????????????????????????????????????
    return new;
}

Bplus_PAN *CreateBplus_PAN(bool leaf){
    Bplus_PAN *new = (Bplus_PAN *)malloc(sizeof(Bplus_PAN));
    new->Isleaf=leaf;
    if(leaf){
        new->Node.LeafNode=CreateLeafPAN();
    } 
    else{
        new->Node.InternalNode.keysize=1;
        new->Node.InternalNode.children[0] = CreateBplus_PAN(True);
        new->Node.InternalNode.children[1] = CreateBplus_PAN(True);
        new->Node.InternalNode.children[0]->Node.LeafNode->prev=NULL;
        new->Node.InternalNode.children[1]->Node.LeafNode->prev = new->Node.InternalNode.children[0]->Node.LeafNode;
        new->Node.InternalNode.children[0]->Node.LeafNode->next = new->Node.InternalNode.children[1]->Node.LeafNode;
        new->Node.InternalNode.children[1]->Node.LeafNode->prev = new->Node.InternalNode.children[0]->Node.LeafNode;
        new->Node.InternalNode.children[1]->Node.LeafNode->next = NULL;
        
        
    }
    return new;

}

//STACK IMPLEMENTATION
//typedef Bplus_PAN * item_type;

typedef struct PANNode{
    Bplus_PAN *data;
    struct PANNode* next;
}PANNode;


typedef struct PANStack{
    PANNode* Top;
}PANStack;

PANNode* CreatePANNode(Bplus_PAN *d){
    PANNode *ptr;
    ptr = (PANNode *)malloc(sizeof(PANNode));
    ptr->data=d;
    ptr->next=NULL;
    return ptr;
}

void InitializePANStack(PANStack *stptr){
    stptr->Top=NULL;
}

bool IsPANStackEmpty(PANStack s){
    return(s.Top==NULL);
}

status_code PushPAN(PANStack *stptr, Bplus_PAN *d){
    status_code sc=Success;
    PANNode *ptr;
    
    ptr = CreatePANNode(d);
    //printf("Push\n");
    if(ptr == NULL) sc=Failure;
    else{
        ptr->next = stptr->Top;
        stptr->Top=ptr;
    }
    return sc;
}

status_code PopPAN(PANStack *stptr, Bplus_PAN** dptr){
    status_code sc= Success;
    if(IsPANStackEmpty(*stptr)) sc=Failure;
    else{
        PANNode *ptr;
        ptr = stptr->Top;
        *dptr = ptr->data;
        stptr->Top = ptr->next;
        free(ptr);
    }
    return sc;
}

Bplus_PAN *FindPANKeyNode(Bplus_PAN *root, int key){
    int nkey = key;
    Bplus_PAN *tptr = root;
    if(tptr ==NULL) return NULL;
    while(tptr->Isleaf!=True){
        int no_children = tptr->Node.InternalNode.keysize+1;
        int i=0;
        int *keys = tptr->Node.InternalNode.keys;
        no_children--;
        while(no_children--){
            if(nkey>=keys[i]){
                i++;
            }
            else no_children=0;
        }
        tptr = tptr->Node.InternalNode.children[i];      
    }
    return tptr;
}

Bplus_PAN *InsertInternalBplusPAN(PANStack *pstack, keytype key, Bplus_PAN *ptr, Bplus_PAN *root){
    if(IsPANStackEmpty(*pstack)){
        //New root
        //printf("Root splitting\n");
        Bplus_PAN *new = CreateBplus_PAN(False);
        new->Node.InternalNode.keysize=1;
        //Linking not done

        new->Node.InternalNode.keys[0]=key;
        new->Node.InternalNode.children[0] = root;
        new->Node.InternalNode.children[1]= ptr;
        return new;
    }
    else{
        Bplus_PAN *parent;
        status_code sc = PopPAN(pstack, &parent);
        if(!sc) printf("FAILURE");
        parent->Node.InternalNode.keysize++;
        int size = parent->Node.InternalNode.keysize;
        keytype arr[size];
        int j=0;
        int childindex;
        int inserted=0;
        for(int i=0;i<size;i++){
            if(!inserted && (j== size-1 || key<parent->Node.InternalNode.keys[j])){
                arr[i] = key;
                childindex= i+1;
                inserted++;
            }
            else{
                arr[i] = parent->Node.InternalNode.keys[j];
                j++;
            }
        }
        for(int i=0;i<size;i++){
            parent->Node.InternalNode.keys[i] = arr[i];
        }
        //Now updating children array
        //printf("childindex: %d, size: %d\n",childindex, size);
        for(int i=size;i>childindex;i--){
            parent->Node.InternalNode.children[i] = parent->Node.InternalNode.children[i-1];
        }
        parent->Node.InternalNode.children[childindex] = ptr;
        
        if(parent->Node.InternalNode.keysize>=ORDER){
            //printf("Splitting internal\n");
            //Split
            Bplus_PAN *newnode = CreateBplus_PAN(False);
            newnode->Node.InternalNode.keysize = (ORDER-1)/2;
            parent->Node.InternalNode.keysize = (ORDER-1)/2;
            for(int i=0;i<(ORDER-1)/2;i++){
                newnode->Node.InternalNode.keys[i] = parent->Node.InternalNode.keys[i+(ORDER+1)/2];
                newnode->Node.InternalNode.children[i] = parent->Node.InternalNode.children[i+(ORDER+1)/2];
                //printf("added key: %d\n",parent->Node.InternalNode.keys[i+(ORDER+1)/2]);
            }
            newnode->Node.InternalNode.children[(ORDER-1)/2] = parent->Node.InternalNode.children[ORDER];
            int key = parent->Node.InternalNode.keys[(ORDER-1)/2];
            //InsertInternal
            return InsertInternalBplusPAN(pstack, key, newnode, root);
        }
        else{
            //return root by popping
            Bplus_PAN *root=parent;
            //printf("no split\n");
            while(!IsPANStackEmpty(*pstack)){
                PopPAN(pstack, &root);
                //printf("popped %d\n",root->Node.InternalNode.keys[0]);
            }
            //printf("root: %d\n",root->Node.InternalNode.keys[0]);
            return root;
        }
    }
}

Bplus_PAN *InsertLeafBplusPAN(Bplus_PAN *root, PAN_Data new_d){
    //Check for root null condition
    if(root==NULL){
        //printf("Inserting at root\n");
        root = CreateBplus_PAN(False);
        root->Node.InternalNode.keysize=1;
        root->Node.InternalNode.keys[0]= new_d.PAN_no;
        root->Node.InternalNode.children[0]->Node.LeafNode->num_vals=0;
        root->Node.InternalNode.children[1]->Node.LeafNode->PAN_vals[0]= new_d;
        root->Node.InternalNode.children[1]->Node.LeafNode->num_vals=1;
        return root;

    }
    int nkey = new_d.PAN_no;
    PANStack ParentStack;
    Bplus_PAN *tptr = root;
    InitializePANStack(&ParentStack);
    while(tptr->Isleaf!=True){
        //printf("total keys %d\n", tptr->Node.InternalNode.keysize);
        PushPAN(&ParentStack, tptr);
        
        int no_children = tptr->Node.InternalNode.keysize+1;
        int i=0;
        int *keys = tptr->Node.InternalNode.keys;
        no_children--;
        while(no_children--){
            if(nkey>=keys[i]){
                i++;
            }
            else no_children=0;
        }
        //printf("Chosen %d\n",i);
        tptr = tptr->Node.InternalNode.children[i];      
    }
    int numval = tptr->Node.LeafNode->num_vals;
    //printf("numval %d\n",numval);
    PAN_Data *PAN_arr = tptr->Node.LeafNode->PAN_vals;
    int j=0;
    //printf("%d is key\n",nkey);
    PAN_Data arr[numval+1];
    int inserted=0;
    for(int i=0;i<numval+1;i++){
        if(!inserted && (j==numval || nkey < tptr->Node.LeafNode->PAN_vals[j].PAN_no)){
            arr[i] = new_d;
            inserted++;
        }
        else{
            arr[i] = tptr->Node.LeafNode->PAN_vals[j];
            j++;
        }
    }
    for(int i=0;i<numval+1;i++){
        tptr->Node.LeafNode->PAN_vals[i] = arr[i];
    }
    
    tptr->Node.LeafNode->num_vals = numval+1;
    for(int i=0;i<numval+1;i++){
        tptr->Node.LeafNode->PAN_vals[i] = arr[i];
    }
    numval++;

    if(numval==ORDER+1){
        //printf("External splitting\n");
        //SPLITTING REQUIRED
        Bplus_PAN *new = CreateBplus_PAN(True);
        new->Node.LeafNode->num_vals=(ORDER+1)/2;
        tptr->Node.LeafNode->num_vals = (ORDER+1)/2;
        int split = (ORDER+1)/2;
        for(int i=0;i<split;i++){
            new->Node.LeafNode->PAN_vals[i] = tptr->Node.LeafNode->PAN_vals[i+split];
        }
        int splitkey = new->Node.LeafNode->PAN_vals[0].PAN_no;
        root = InsertInternalBplusPAN(&ParentStack,splitkey, new, root);
        
        //Updating the dll
        if(tptr->Node.LeafNode!=NULL){
            PANleaf *next= tptr->Node.LeafNode->next;
            if(next==NULL){
                tptr->Node.LeafNode->next = new->Node.LeafNode;
                new->Node.LeafNode->prev = tptr->Node.LeafNode;
            }
            else{

                tptr->Node.LeafNode->next = new->Node.LeafNode;
                new->Node.LeafNode->prev = tptr->Node.LeafNode;
                new->Node.LeafNode->next = next;
                next->prev = new->Node.LeafNode;
            }
        }



    }
    return root;
    
}

void PrintBplusPANTree(Bplus_PAN *root) {
    if (root == NULL) {
        printf("Empty tree\n");
        return;
    }

    Bplus_PAN *queue[1000];
    int front = 0, rear = 0;
    queue[rear++] = root;

    while (front < rear) {
        int currentLevelSize = rear - front;
        for (int i = 0; i < currentLevelSize; i++) {
            //printf("i: %d",i);
            Bplus_PAN *currentNode = queue[front++];
            //printf("Hello\n");
            if (currentNode->Isleaf == True) {
                PANleaf *leaf = currentNode->Node.LeafNode;
                printf("[ ");
                printf("Leafsize: %d|| ",leaf->num_vals);
                for (int j = 0; j < leaf->num_vals; j++) {
                    printf("%d:%s ", leaf->PAN_vals[j].PAN_no, leaf->PAN_vals[j].Name);
                }
                printf("] ");
            } 
            else {
                printf("[ ");
                for (int j = 0; j < currentNode->Node.InternalNode.keysize; j++) {
                    printf("%d ", currentNode->Node.InternalNode.keys[j]);
                }
                printf("] ");

                for (int j = 0; j <= currentNode->Node.InternalNode.keysize; j++) {
                    queue[rear++] = currentNode->Node.InternalNode.children[j];
                }
            }
        }
        printf("\n");
    }
}

///////////////////////////////////////////////////////LPG DATA////////////////////////////////////////

typedef struct LPG_node{
    char Name[100];
    int acc_no;
    lpg_stat taken;

}LPG_node;

LPG_node *CreateLPGNode(char *name, int accnum, lpg_stat take){
    LPG_node *new= (LPG_node *)malloc(sizeof(LPG_node));
    new->acc_no = accnum;
    strcpy(new->Name,name);
    new->taken =take;
    return new;
}


///////////////////////////////////////////////////////AADHAR DEFINITIONS///////////////////////////////////////////////////////////

typedef struct AadharData{
    char Name[100];
    char Address[200];
    int Aadhar_no;
    Bplus_PAN *PAN_DB;
    LPG_node *LPG_info;
    
    
}Aadhar_Data;

//typedef Aadhar_Data leafdata;
typedef struct Aadharleaf{
    Aadhar_Data Aadhar_vals[ORDER+1];
    int num_vals;
    struct Aadharleaf *next;
    struct Aadharleaf *prev;
} Aadharleaf;

//typedef Aadharleaf * leaftype;

typedef struct Internal_Aadhar{
    struct Bplus_Aadhar *children[ORDER+1];
    keytype keys[ORDER];
    int keysize; //current number of keys
}Internal_Aadhar;

typedef struct Bplus_Aadhar{
    bool Isleaf;
    union{
        Internal_Aadhar InternalNode;
        Aadharleaf *LeafNode;
    } Node;
    
}Bplus_Aadhar;

Aadharleaf *CreateLeafAadhar(){
    Aadharleaf *new = (Aadharleaf *)malloc(sizeof(Aadharleaf));
    new->next=NULL;
    new->prev=NULL;
    new->num_vals = 0;
    for(int i=0;i<ORDER;i++){
        new->Aadhar_vals[i].PAN_DB=NULL;
        new->Aadhar_vals[i].LPG_info=NULL;
    }
    
    return new;
}

Bplus_Aadhar *CreateBplus_Aadhar(bool leaf){
    Bplus_Aadhar *new = (Bplus_Aadhar *)malloc(sizeof(Bplus_Aadhar));
    new->Isleaf=leaf;
    if(leaf){
        new->Node.LeafNode=CreateLeafAadhar();
    } 
    else{
        new->Node.InternalNode.keysize=1;
        new->Node.InternalNode.children[0] = CreateBplus_Aadhar(True);
        new->Node.InternalNode.children[1] = CreateBplus_Aadhar(True);
        new->Node.InternalNode.children[0]->Node.LeafNode->prev=NULL;
        new->Node.InternalNode.children[1]->Node.LeafNode->prev = new->Node.InternalNode.children[0]->Node.LeafNode;
        new->Node.InternalNode.children[0]->Node.LeafNode->next = new->Node.InternalNode.children[1]->Node.LeafNode;
        new->Node.InternalNode.children[1]->Node.LeafNode->prev = new->Node.InternalNode.children[0]->Node.LeafNode;
        new->Node.InternalNode.children[1]->Node.LeafNode->next = NULL;
        
        
    }
    return new;

}



//STACK IMPLEMENTATION

typedef struct NodeTag{
    Bplus_Aadhar *data;
    struct NodeTag* next;
}AadharNode;


typedef struct StackAadhar{
    AadharNode* Top;
}AadharStack;

AadharNode* CreateAadharNode(Bplus_Aadhar *d){
    AadharNode *ptr;
    ptr = (AadharNode *)malloc(sizeof(AadharNode));
    ptr->data=d;
    ptr->next=NULL;
    return ptr;
}

void InitializeAadharStack(AadharStack *stptr){
    stptr->Top=NULL;
}

bool IsAadharStackEmpty(AadharStack s){
    return(s.Top==NULL);
}

status_code PushAadhar(AadharStack *stptr, Bplus_Aadhar *d){
    status_code sc=Success;
    AadharNode *ptr;
    
    ptr = CreateAadharNode(d);
    //printf("Push\n");
    if(ptr == NULL) sc=Failure;
    else{
        ptr->next = stptr->Top;
        stptr->Top=ptr;
    }
    return sc;
}

status_code PopAadhar(AadharStack *stptr, Bplus_Aadhar **dptr){
    status_code sc= Success;
    if(IsAadharStackEmpty(*stptr)) sc=Failure;
    else{
        AadharNode *ptr;
        ptr = stptr->Top;
        *dptr = ptr->data;
        stptr->Top = ptr->next;
        free(ptr);
    }
    return sc;
}


Bplus_Aadhar *FindAadharkeyNode(Bplus_Aadhar *root, int key){
    int nkey = key;
    Bplus_Aadhar *tptr = root;
    if(tptr==NULL) return NULL;
    while(tptr->Isleaf!=True){
        int no_children = tptr->Node.InternalNode.keysize+1;
        int i=0;
        int *keys = tptr->Node.InternalNode.keys;
        no_children--;
        while(no_children--){
            if(nkey>=keys[i]){
                i++;
            }
            else no_children=0;
        }
        tptr = tptr->Node.InternalNode.children[i];      
    }
    return tptr;
}

Bplus_Aadhar *InsertInternalBplusAadhar(AadharStack *pstack, keytype key, Bplus_Aadhar *ptr, Bplus_Aadhar *root){
    if(IsAadharStackEmpty(*pstack)){
        //New root
        //printf("Root splitting\n");
        Bplus_Aadhar *new = CreateBplus_Aadhar(False);
        new->Node.InternalNode.keysize=1;
        //Linking not done

        new->Node.InternalNode.keys[0]=key;
        new->Node.InternalNode.children[0] = root;
        new->Node.InternalNode.children[1]= ptr;
        return new;
    }
    else{
        Bplus_Aadhar *parent;
        status_code sc = PopAadhar(pstack, &parent);
        if(!sc) printf("FAILURE");
        parent->Node.InternalNode.keysize++;
        int size = parent->Node.InternalNode.keysize;
        //wrong: Insert at correct position of the key array and ptr array
        /*parent->Node.InternalNode.children[size] = ptr;
        parent->Node.InternalNode.keys[size-1] = key;*/
        keytype arr[size];
        int j=0;
        int childindex;
        int inserted=0;
        for(int i=0;i<size;i++){
            if(!inserted && (j== size-1 || key<parent->Node.InternalNode.keys[j])){
                arr[i] = key;
                childindex= i+1;
                inserted++;
            }
            else{
                arr[i] = parent->Node.InternalNode.keys[j];
                j++;
            }
        }
        for(int i=0;i<size;i++){
            parent->Node.InternalNode.keys[i] = arr[i];
        }
        //Now updating children array
        //printf("childindex: %d, size: %d\n",childindex, size);
        for(int i=size;i>childindex;i--){
            parent->Node.InternalNode.children[i] = parent->Node.InternalNode.children[i-1];
        }
        parent->Node.InternalNode.children[childindex] = ptr;
        
        if(parent->Node.InternalNode.keysize>=ORDER){
            //printf("Splitting internal\n");
            //Split
            Bplus_Aadhar *newnode = CreateBplus_Aadhar(False);
            newnode->Node.InternalNode.keysize = (ORDER-1)/2;
            parent->Node.InternalNode.keysize = (ORDER-1)/2;
            for(int i=0;i<(ORDER-1)/2;i++){
                newnode->Node.InternalNode.keys[i] = parent->Node.InternalNode.keys[i+(ORDER+1)/2];
                newnode->Node.InternalNode.children[i] = parent->Node.InternalNode.children[i+(ORDER+1)/2];
                //printf("added key: %d\n",parent->Node.InternalNode.keys[i+(ORDER+1)/2]);
            }
            newnode->Node.InternalNode.children[(ORDER-1)/2] = parent->Node.InternalNode.children[ORDER];
            int key = parent->Node.InternalNode.keys[(ORDER-1)/2];
            //InsertInternal
            return InsertInternalBplusAadhar(pstack, key, newnode, root);
        }
        else{
            //return root by popping
            Bplus_Aadhar *root=parent;
            //printf("no split\n");
            while(!IsAadharStackEmpty(*pstack)){
                PopAadhar(pstack, &root);
                //printf("popped %d\n",root->Node.InternalNode.keys[0]);
            }
            //printf("root: %d\n",root->Node.InternalNode.keys[0]);
            return root;
        }
    }
}

Bplus_Aadhar *InsertLeafBplusAadhar(Bplus_Aadhar *root, Aadhar_Data new_d){
    //Check for root null condition
    if(root==NULL){
        root = CreateBplus_Aadhar(False);
        root->Node.InternalNode.keysize=1;
        root->Node.InternalNode.keys[0]= new_d.Aadhar_no;
        root->Node.InternalNode.children[0]->Node.LeafNode->num_vals=0;
        root->Node.InternalNode.children[1]->Node.LeafNode->Aadhar_vals[0]= new_d;
        root->Node.InternalNode.children[1]->Node.LeafNode->num_vals=1;
        return root;

    }
    int nkey = new_d.Aadhar_no;
    AadharStack ParentStack;
    Bplus_Aadhar *tptr = root;
    InitializeAadharStack(&ParentStack);
    while(tptr->Isleaf!=True){
        //printf("total keys %d\n", tptr->Node.InternalNode.keysize);
        PushAadhar(&ParentStack, tptr);
        
        int no_children = tptr->Node.InternalNode.keysize+1;
        int i=0;
        int *keys = tptr->Node.InternalNode.keys;
        no_children--;
        while(no_children--){
            if(nkey>=keys[i]){
                i++;
            }
            else no_children=0;
        }
        //printf("Chosen %d\n",i);
        tptr = tptr->Node.InternalNode.children[i];      
    }
    int numval = tptr->Node.LeafNode->num_vals;
    //printf("numval %d\n",numval);
    Aadhar_Data *aadhar_arr = tptr->Node.LeafNode->Aadhar_vals;
    int j=0;
    //printf("%d is key\n",nkey);
    Aadhar_Data arr[numval+1];
    int inserted=0;
    for(int i=0;i<numval+1;i++){
        if(!inserted && (j==numval || nkey < tptr->Node.LeafNode->Aadhar_vals[j].Aadhar_no)){
            arr[i] = new_d;
            inserted++;
        }
        else if(nkey == tptr->Node.LeafNode->Aadhar_vals[j].Aadhar_no){
            printf("Error: This Aadhar already exists\n");
            return root;
        }
        else{
            arr[i] = tptr->Node.LeafNode->Aadhar_vals[j];
            j++;
        }
    }
    for(int i=0;i<numval+1;i++){
        tptr->Node.LeafNode->Aadhar_vals[i] = arr[i];
    }
    
    tptr->Node.LeafNode->num_vals = numval+1;
    for(int i=0;i<numval+1;i++){
        tptr->Node.LeafNode->Aadhar_vals[i] = arr[i];
    }
    numval++;

    if(numval==ORDER+1){
        //printf("External splitting\n");
        //SPLITTING REQUIRED
        Bplus_Aadhar *new = CreateBplus_Aadhar(True);
        new->Node.LeafNode->num_vals=(ORDER+1)/2;
        tptr->Node.LeafNode->num_vals = (ORDER+1)/2;
        int split = (ORDER+1)/2;
        for(int i=0;i<split;i++){
            new->Node.LeafNode->Aadhar_vals[i] = tptr->Node.LeafNode->Aadhar_vals[i+split];
        }
        int splitkey = new->Node.LeafNode->Aadhar_vals[0].Aadhar_no;
        root = InsertInternalBplusAadhar(&ParentStack,splitkey, new, root);
        
        //Updating the dll
        if(tptr->Node.LeafNode!=NULL){
            Aadharleaf *next= tptr->Node.LeafNode->next;
            if(next==NULL){
                tptr->Node.LeafNode->next = new->Node.LeafNode;
                new->Node.LeafNode->prev = tptr->Node.LeafNode;
            }
            else{

                tptr->Node.LeafNode->next = new->Node.LeafNode;
                new->Node.LeafNode->prev = tptr->Node.LeafNode;
                new->Node.LeafNode->next = next;
                next->prev = new->Node.LeafNode;
            }
        }



    }
    return root;
    
}

void PrintBplusTree2(Bplus_Aadhar *root){
    Bplus_Aadhar *list = FindAadharkeyNode(root, INT_MIN);
    Aadharleaf *leaflist = list->Node.LeafNode;
    while(leaflist!=NULL){
        printf("[ ");
        for(int i=0;i<leaflist->num_vals;i++){
            printf("%d ",leaflist->Aadhar_vals[i].Aadhar_no);
        }
        printf(" ]  ");
        leaflist = leaflist->next;
    }

}


void PrintBplusAadharTree(Bplus_Aadhar *root) {
    if (root == NULL) {
        printf("Empty tree\n");
        return;
    }

    Bplus_Aadhar *queue[1000];
    int front = 0, rear = 0;
    queue[rear++] = root;

    while (front < rear) {
        int currentLevelSize = rear - front;
        for (int i = 0; i < currentLevelSize; i++) {
            //printf("i: %d",i);
            Bplus_Aadhar *currentNode = queue[front++];
            //printf("Hello\n");
            if (currentNode->Isleaf == True) {
                Aadharleaf *leaf = currentNode->Node.LeafNode;
                printf("[ ");
                printf("Leafsize: %d ",leaf->num_vals);
                for (int j = 0; j < leaf->num_vals; j++) {
                    printf("%d:%s ", leaf->Aadhar_vals[j].Aadhar_no, leaf->Aadhar_vals[j].Name);
                }
                printf("] ");
            } 
            else {
                printf("[ ");
                for (int j = 0; j < currentNode->Node.InternalNode.keysize; j++) {
                    printf("%d ", currentNode->Node.InternalNode.keys[j]);
                }
                printf("] ");

                for (int j = 0; j <= currentNode->Node.InternalNode.keysize; j++) {
                    queue[rear++] = currentNode->Node.InternalNode.children[j];
                }
            }
        }
        printf("\n");
    }
}

//////////////////FUNCTIONS FOR THE QUESTIONS////////////////

Bplus_Aadhar *InsertNewAadhar(Bplus_Aadhar *root, Aadhar_Data newd){
    return InsertLeafBplusAadhar(root, newd);
}

status_code InsertNewPAN(Bplus_Aadhar *root, PAN_Data newd){
    //printf("Inserting PAN %d with aadhar %d\n", newd.PAN_no, newd.Aadhar_num);
    status_code sc = Success;
    int aadharno = newd.Aadhar_num;
    Bplus_Aadhar *searchleaf = FindAadharkeyNode(root, aadharno);
    int numval = searchleaf->Node.LeafNode->num_vals;
    int found=0;
    int i=0;
    while(i<numval && !found){
        //printf("%d aadhar\n",searchleaf->Node.LeafNode->Aadhar_vals[i].Aadhar_no);
        if(searchleaf->Node.LeafNode->Aadhar_vals[i].Aadhar_no==aadharno){
            found++;
        }  
        else i++;
    }
    if(!found) sc= Failure;
    else searchleaf->Node.LeafNode->Aadhar_vals[i].PAN_DB = InsertLeafBplusPAN(searchleaf->Node.LeafNode->Aadhar_vals[i].PAN_DB, newd);
    return sc;
}

status_code InsertNewBank(Bplus_Aadhar *root, Bank_Data newd){
    //printf("Insert Bank");
    status_code sc = Failure;
    int pannum = newd.Pan_no;
    Bplus_Aadhar *leftmost = FindAadharkeyNode(root, INT_MIN);
    Aadharleaf *aadharlist = leftmost->Node.LeafNode;
    int i=0;
    int j=0;
    while(aadharlist!=NULL && sc==Failure){
        //Check each aadhar leaf
        i=0;
        while(i<aadharlist->num_vals && !sc){
            //Check each aadhar card
            //Log search for probable PAN card from that Aadhar Card
            Bplus_PAN *searchpan = FindPANKeyNode(aadharlist->Aadhar_vals[i].PAN_DB, pannum);
            j=0;
            if(searchpan!=NULL){
                while(j<searchpan->Node.LeafNode->num_vals && !sc){
                    if(searchpan->Node.LeafNode->PAN_vals[j].PAN_no == pannum){
                        sc = Success;
                        searchpan->Node.LeafNode->PAN_vals[j].Bank_DB = InsertLeafBplusBank(searchpan->Node.LeafNode->PAN_vals[j].Bank_DB, newd);
                    }
                    else j++;
                }
            }
            
            if(!sc) i++;

        }
        if(!sc) aadharlist = aadharlist->next;
    }
    return sc;
}

status_code InsertNewLPG(Bplus_Aadhar *root, LPG_node *lpgnode){
    status_code sc;
    int bankaccno = lpgnode->acc_no;
    //printf("Inserting LPG for bank acc.: %d\n",bankaccno);
    Bplus_Aadhar *leftmost = FindAadharkeyNode(root, INT_MIN);
    Aadharleaf *aadharlist = leftmost->Node.LeafNode;
    
    int found=0;
    while(aadharlist!=NULL && !found){    
        int i=0;
        while(i<aadharlist->num_vals && !found){
            if(aadharlist->Aadhar_vals[i].LPG_info==NULL){
                Bplus_PAN *leftmostPAN = FindPANKeyNode(aadharlist->Aadhar_vals[i].PAN_DB, INT_MIN);
                if(leftmostPAN!=NULL){
                    PANleaf *panlist = leftmostPAN->Node.LeafNode;
                    while(panlist!=NULL && !found){
                        int j=0;
                        while(j<panlist->num_vals && !found){
                            Bplus_Bank *leftmostBank = FindBankKeyNode(panlist->PAN_vals[j].Bank_DB, INT_MIN);
                            if(leftmostBank!=NULL){
                                Bankleaf *Banklist = leftmostBank->Node.LeafNode;
                                while(Banklist!=NULL && !found){
                                    int k=0;
                                    while(k<Banklist->num_vals && !found){
                                        if(Banklist->Bank_vals[k].Bank_details.Acc_no == bankaccno){
                                            found++;
                                            aadharlist->Aadhar_vals[i].LPG_info = lpgnode;
                                            //printf("found %d\n",Banklist->Bank_vals[k].Bank_details.Acc_no);
                                        }
                                        else if(!found) k++;
                                    }
                                    if(!found) Banklist = Banklist->next;
                                }
                                
                            }
                            if(!found) j++;
                            
                        }
                        if(!found) panlist = panlist->next;
                    }
                }
                if(!found) i++;
            }
            else i++;
        }
        aadharlist = aadharlist->next;
    }
    if(found) sc = Success;
    else sc = Failure;
    return sc;
}

void PrintAadharinfo(Aadhar_Data aadhar)
{
    printf("Name: %s\nAddress: %s\nAadhaar no.: %d\n", aadhar.Name, aadhar.Address, aadhar.Aadhar_no);
    printf("||||||||||||||||||||||||\n");
}

void PrintPANinfo(PAN_Data pan)
{
    printf("Name: %s\nAddress: %s\nAadhaar no.: %d\nPAN no.: %d\n", pan.Name, pan.Address, pan.Aadhar_num, pan.PAN_no);
    printf("||||||||||||||||||||||||\n");
}

void PrintLPGinfo(LPG_node *lpgnode)
{
    printf("Name: %s\nAccount no.: %d\nTaken: %d\n", lpgnode->Name, lpgnode->acc_no, lpgnode->taken);
    printf("||||||||||||||||||||||||\n");
}

void PrintBankinfo(Bank_Data bank)
{
    printf("Name: %s\nAddress: %s\nBank Name: %s\nAccount no.:%d\nAmount:%f\n", bank.Name, bank.Address,bank.Bank_details.BankName, bank.Bank_details.Acc_no, bank.Bank_details.deposited);
    printf("||||||||||||||||||||||||\n");
}

void Print_PANless_Aadhar(Bplus_Aadhar *database) //QUESTION 1
{
    printf("PRINTING AADHAR WITH NO PAN CARDS\n");
    Bplus_Aadhar *leftmost = FindAadharkeyNode(database, INT_MIN);
    Aadharleaf *aadharleaves = leftmost->Node.LeafNode;
    while(aadharleaves!=NULL){
        for(int i=0;i<aadharleaves->num_vals;i++){
            if(aadharleaves->Aadhar_vals[i].PAN_DB == NULL) PrintAadharinfo(aadharleaves->Aadhar_vals[i]);
        }
        aadharleaves = aadharleaves->next;
    }

}

void Print_MULPAN_Aadhar(Bplus_Aadhar *database) //QUESTION 2
{
    printf("PRINTING AADHAR WITH MULTIPLE PAN CARDS\n");
    Bplus_Aadhar *leftmost = FindAadharkeyNode(database, INT_MIN);
    Aadharleaf *aadharleaves = leftmost->Node.LeafNode;
    while(aadharleaves!=NULL){
        for(int i=0;i<aadharleaves->num_vals;i++){
            int numpans=0;
            Bplus_PAN *leftpan = FindPANKeyNode(aadharleaves->Aadhar_vals[i].PAN_DB, INT_MIN);
            if(leftpan!=NULL){
                PANleaf *panleaves = leftpan->Node.LeafNode;
                while(numpans<2 && panleaves!=NULL)
                {
                    numpans += panleaves->num_vals;
                    panleaves = panleaves->next;
                }
                if(numpans>1){
                    PrintAadharinfo(aadharleaves->Aadhar_vals[i]);
                    Bplus_PAN *leftmostpan = FindPANKeyNode(aadharleaves->Aadhar_vals[i].PAN_DB, INT_MIN);
                    PANleaf *list = leftmostpan->Node.LeafNode;
                    while(list!=NULL){
                        for(int k=0;k<list->num_vals;k++){
                            printf("PAN NO.: %d\n",list->PAN_vals[k].PAN_no);
                        }
                        list = list->next;
                    }
                    printf("-------------------\n");


                } 
            }
            
        }
        aadharleaves = aadharleaves->next;
    }
}

void Print_MulBank_Aadhar(Bplus_Aadhar *database) //QUESTION 3
{
    Bplus_Aadhar *leftmost = FindAadharkeyNode(database, INT_MIN);
    Aadharleaf *aadharleaves = leftmost->Node.LeafNode;
    printf("Printing Aadhar with Multiple Bank accounts\n");
    while(aadharleaves!=NULL)
    {
        for(int i=0;i<aadharleaves->num_vals;i++){
            int counts=0;
            Bplus_PAN *leftpan = FindPANKeyNode(aadharleaves->Aadhar_vals[i].PAN_DB, INT_MIN);
            if(leftpan!=NULL){
                PANleaf *panleaves = leftpan->Node.LeafNode;
                while(panleaves!=NULL && counts<2)
                {
                    int j=0;
                    while(j<panleaves->num_vals && counts<2){
                        if(panleaves->PAN_vals[j].Bank_DB!= NULL) counts++;
                        j++;                    
                    }
                    if(counts<2) panleaves = panleaves->next;

                }
            }
            
            if(counts==2) PrintAadharinfo(aadharleaves->Aadhar_vals[i]);

        }
        aadharleaves = aadharleaves->next;
        
        
    }
}

void Print_LPG_taken_details(Bplus_Aadhar *database) //QUESTION 4
{
    Bplus_Aadhar *leftmost = FindAadharkeyNode(database, INT_MIN);
    Aadharleaf *aadharleaves = leftmost->Node.LeafNode;
    printf("~~~~~LIST OF PEOPLE WHO AVAILED LPG SUBSIDY~~~~~ \n");
    while(aadharleaves!=NULL){
        for(int i=0;i<aadharleaves->num_vals;i++){
            if(aadharleaves->Aadhar_vals[i].LPG_info!=NULL){
                if(aadharleaves->Aadhar_vals[i].LPG_info->taken){
                    printf("Printing Aadhar info of the person");
                    PrintAadharinfo(aadharleaves->Aadhar_vals[i]);
                    Bplus_PAN *leftpan = FindPANKeyNode(aadharleaves->Aadhar_vals[i].PAN_DB, INT_MIN);
                    if(leftpan!=NULL){
                        printf("Printing the PAN details:\n");
                        PANleaf *PANleaves = leftpan->Node.LeafNode;
                        while(PANleaves!=NULL){
                            for(int j=0;j<PANleaves->num_vals;j++){
                                PrintPANinfo(PANleaves->PAN_vals[j]);
                                printf("The bank accounts registered under this are:\n");
                                Bplus_Bank *leftbank = FindBankKeyNode(PANleaves->PAN_vals[j].Bank_DB, INT_MIN);
                                if(leftbank!=NULL){
                                    Bankleaf *bankleaves = leftbank->Node.LeafNode;
                                    while(bankleaves!=NULL){
                                        for(int k=0;k<bankleaves->num_vals;k++){
                                            PrintBankinfo(bankleaves->Bank_vals[k]);
                                        }
                                        bankleaves = bankleaves->next;
                                    }
                                } 

                            }
                            PANleaves = PANleaves->next;
                        }
                    }
                }
            }
            
        }
        aadharleaves = aadharleaves->next;
    }

}

void Print_GreaterThanAmount(Bplus_Aadhar *database, float x)  //Question 5
{
    printf("Printing details of people with more than %f balance And have an LPG subsidy\n",x);
    Bplus_Aadhar *leftmost = FindAadharkeyNode(database, INT_MIN);
    Aadharleaf *aadharleaves = leftmost->Node.LeafNode;
    while(aadharleaves!=NULL)
    {
        for(int i=0;i<aadharleaves->num_vals;i++){
            if(aadharleaves->Aadhar_vals[i].LPG_info!=NULL && aadharleaves->Aadhar_vals[i].LPG_info->taken){
                float amount=0;
                Bplus_PAN *leftpan = FindPANKeyNode(aadharleaves->Aadhar_vals[i].PAN_DB, INT_MIN);
                if(leftpan!=NULL){
                    PANleaf *panleaves = leftpan->Node.LeafNode;
                    while(panleaves!=NULL && amount<=x)
                    {
                        int j=0;
                        while(j<panleaves->num_vals && amount<=x){
                            Bplus_Bank *leftbank = FindBankKeyNode(panleaves->PAN_vals[j].Bank_DB, INT_MIN);
                            Bankleaf *bankleaves = leftbank->Node.LeafNode;
                            while(bankleaves!=NULL && amount<=x){
                                int k=0;
                                while(k<bankleaves->num_vals && amount<=x){
                                    amount = amount + bankleaves->Bank_vals[k].Bank_details.deposited;
                                    k++;
                                }
                                if(amount<=x) bankleaves = bankleaves->next;
                            }
                            if(amount<=x) j++;
                        }
                        if(amount<=x) panleaves= panleaves->next;
                    }
                }
                if(amount>x) PrintAadharinfo(aadharleaves->Aadhar_vals[i]);
            }
            
            
            
        }
        aadharleaves = aadharleaves->next;
        
    }
}
//QUESTION 6
void PrintInconsistent_Data(Bplus_Aadhar *database)
{
    printf("PRINTING INCONSISTENT DATA\n");
    Bplus_Aadhar *leftmost = FindAadharkeyNode(database, INT_MIN);
    Aadharleaf *aadharleaves = leftmost->Node.LeafNode;
    while(aadharleaves!=NULL)
    {
        for(int i=0;i<aadharleaves->num_vals;i++){

            char realname[100];
            strcpy(realname, aadharleaves->Aadhar_vals[i].Name);
            printf("THE REAL NAME: %s\n",realname);

            if(aadharleaves->Aadhar_vals[i].LPG_info!=NULL && strcmp(realname, aadharleaves->Aadhar_vals[i].LPG_info->Name)!=0){
                printf("Inconsistent LPG details\n");
                PrintLPGinfo(aadharleaves->Aadhar_vals[i].LPG_info);
            }
            
            Bplus_PAN *leftpan = FindPANKeyNode(aadharleaves->Aadhar_vals[i].PAN_DB, INT_MIN);
            if(leftpan!=NULL){
                PANleaf *panleaves = leftpan->Node.LeafNode;
                while(panleaves!=NULL)
                {

                    int j=0;
                    while(j<panleaves->num_vals){

                        if(strcmp(realname, panleaves->PAN_vals[j].Name)!=0){
                            printf("PAN Inconsistent:\n");
                            PrintPANinfo(panleaves->PAN_vals[j]);
                        }
                        Bplus_Bank *leftbank = FindBankKeyNode(panleaves->PAN_vals[j].Bank_DB, INT_MIN);
                        if(leftbank!=NULL){
                            Bankleaf *bankleaves = leftbank->Node.LeafNode;
                            while(bankleaves!=NULL){

                                int k=0;
                                while(k<bankleaves->num_vals){

                                    if(strcmp(realname, bankleaves->Bank_vals[k].Name)!=0){
                                        printf("Inconsistent Bank info\n");
                                        PrintBankinfo(bankleaves->Bank_vals[k]);
                                    }
                                    k++;
                                }
                                bankleaves = bankleaves->next;
                            }
                            j++;
                        }
                        else j++;
                        
                    }
                    panleaves= panleaves->next;
                }
            }
            printf("---------Thats all for %s----------\n",aadharleaves->Aadhar_vals[i].Name);
            
        }
        aadharleaves = aadharleaves->next;
        
    }

}

//QUESTION 7
status_code MergeBankList(Bplus_Aadhar *database, Bank_Data *DB1, int size1, Bank_Data *DB2, int size2){
    status_code sc = Success;
    int i=0;
    while(i<size1 && sc){
        sc = InsertNewBank(database, DB1[i]);
        i++;
    }
    i=0;
    while(i<size2 && sc){
        sc = InsertNewBank(database, DB2[i]);
        i++;
    }
    return sc;

}

void RangeSearch(Bplus_Aadhar *database, int aadhar1, int aadhar2) //QUESTION 8
{
    Bplus_Aadhar *left = FindAadharkeyNode(database, aadhar1);
    Aadharleaf *Aadharleaves = left->Node.LeafNode;
    int stop=0;
    while(Aadharleaves!= NULL && !stop){
        int i=0;
        while(i<Aadharleaves->num_vals && !stop){
            if(Aadharleaves->Aadhar_vals[i].Aadhar_no<aadhar1) i++;
            else if(Aadharleaves->Aadhar_vals[i].Aadhar_no<=aadhar2){
                PrintAadharinfo(Aadharleaves->Aadhar_vals[i]);
                i++;
            }
            else stop++;
        }
        if(!stop) Aadharleaves= Aadharleaves->next;
    }
}

int main()
{
    Bplus_Aadhar *Database=NULL;
   
    FILE *filePointer0 = fopen("Aadhar_data.txt", "r");
    if (filePointer0 != NULL)
    {
        char name[100];
        char address[200];
        int aadhar;
        
        while (!feof(filePointer0))
        {
            char buffer[100];
            fscanf(filePointer0, "%[^\n]s", name);
            fgets(buffer, sizeof(buffer), filePointer0);
            fscanf(filePointer0, "%[^\n]s",address);
            fgets(buffer, sizeof(buffer), filePointer0);
            fscanf(filePointer0, "%d",&aadhar);
            fgets(buffer, sizeof(buffer), filePointer0);
            fgets(buffer, sizeof(buffer), filePointer0);
            Aadhar_Data dataaadhar;
            strcpy(dataaadhar.Name, name);
            strcpy(dataaadhar.Address, address);
            strcpy(dataaadhar.Name, name);
            dataaadhar.Aadhar_no = aadhar;
            dataaadhar.LPG_info =NULL;
            dataaadhar.PAN_DB =NULL;

            Database = InsertNewAadhar(Database, dataaadhar);
        }
        
    fclose(filePointer0);
    }
    else printf("Aadhar data empty\n");
    
    FILE *filePointer1 = fopen("PAN_data.txt", "r");
    if (filePointer1 != NULL)
    {
        char name[100];
        char address[200];
        int aadhar;
        int pan;
        while (!feof(filePointer1))
        {
            char buffer[100];
            fscanf(filePointer1, "%[^\n]s", name);
            fgets(buffer, sizeof(buffer), filePointer1);
            fscanf(filePointer1, "%[^\n]s",address);
            fgets(buffer, sizeof(buffer), filePointer1);
            fscanf(filePointer1, "%d",&aadhar);
            fgets(buffer, sizeof(buffer), filePointer1);
            fscanf(filePointer1, "%d", &pan);
            fgets(buffer, sizeof(buffer), filePointer1);
            if(!feof(filePointer1)) fgets(buffer, sizeof(buffer), filePointer1);
            

            //printf("Taken name: %s address: %s aadhar: %d", name, address, aadhar);
            PAN_Data datapan;
            
            strcpy(datapan.Name, name);
            strcpy(datapan.Address, address);
            datapan.PAN_no = pan;
            datapan.Aadhar_num = aadhar;
            datapan.Bank_DB = NULL;
            status_code sc = InsertNewPAN(Database, datapan);
            if(sc == Failure) printf("PAN Update unsuccessful\n");
            else printf("PAN Added from the file\n");
            
        }
        
    fclose(filePointer1);
    }
    else printf("PAN data empty\n");

    

    
    FILE *filePointer2 = fopen("Bank_data.txt", "r");
    if (filePointer2 != NULL)
    {
        char name[100];
        char address[200];
        char Bankname[200];
        int acc,pan;
        float deposit;
        while (!feof(filePointer2))
        {
            char buffer[100];
            fscanf(filePointer2, "%[^\n]s", name);
            fgets(buffer, sizeof(buffer), filePointer2);
            fscanf(filePointer2, "%[^\n]s",address);
            fgets(buffer, sizeof(buffer), filePointer2);
            fscanf(filePointer2, "%d",&pan);
            fgets(buffer, sizeof(buffer), filePointer2);
            fscanf(filePointer2, "%d",&acc);
            fgets(buffer, sizeof(buffer), filePointer2);
            fscanf(filePointer2, "%f", &deposit);
            fgets(buffer, sizeof(buffer), filePointer2);
            fscanf(filePointer2, "%[^\n]s", Bankname);
            fgets(buffer, sizeof(buffer), filePointer2);
            fgets(buffer, sizeof(buffer), filePointer2);

            Bank_Data databank;
            strcpy(databank.Name,name);
            strcpy(databank.Address,address);
            strcpy(databank.Bank_details.BankName,Bankname);
            databank.Bank_details.Acc_no = acc;
            databank.Bank_details.deposited = deposit;
            databank.Pan_no = pan;
            
            status_code sc = InsertNewBank(Database, databank);
            
            if(sc == Failure) printf("Bank Update unsuccessful\n");
            else printf("Bank Added from the file\n");
        }
        
    fclose(filePointer2);
    }
    else printf("Bank data empty\n");
    
    
    
    FILE *filePointer3 = fopen("LPG_data.txt", "r");
    if (filePointer3 != NULL)
    {
        char name[100];
        int acc;
        lpg_stat lpgstatus;
        while (!feof(filePointer3))
        {
            char buffer[100];
            fscanf(filePointer3, "%[^\n]s", name);
            fgets(buffer, sizeof(buffer), filePointer3);
            fscanf(filePointer3, "%d",&acc);
            fgets(buffer, sizeof(buffer), filePointer3);
            fscanf(filePointer3, "%d",&lpgstatus);
            fgets(buffer, sizeof(buffer), filePointer3);
            fgets(buffer, sizeof(buffer), filePointer3);
            

            //printf("Taken name: %s address: %s aadhar: %d", name, address, aadhar);
            LPG_node *new = (LPG_node *)malloc(sizeof(LPG_node));
            strcpy(new->Name, name);
            new->acc_no = acc;
            new->taken = lpgstatus;

            status_code sc = InsertNewLPG(Database, new);
            if(sc == Failure) printf("LPG Update unsuccessful\n");
            else printf("LPG Added from the file\n");
            
        }
        
    fclose(filePointer3);
    }
    else printf("LPG data empty\n");

    PrintBplusAadharTree(Database);
    printf("-------------------------------------------------\n");
    printf("|Enter the task you want to do and 0 to exit:   |\n");
    printf("| 1- Print citizens without PAN                 |\n");
    printf("| 2- Print citizens with multiple PAN           |\n");
    printf("| 3- Print citizens with multiple Bank accounts |\n");
    printf("| 4- Print citizens with LPG subsidy            |\n");
    printf("| 5- Print citizens with total threshold balance|\n");
    printf("| 6- Print citizens with inconsistent data      |\n");
    printf("| 7- Add New Bank dataset                       |\n");
    printf("-------------------------------------------------\n");
    int button=1;
    while(button!=0){
        scanf("%d",&button);
        if(button==1)Print_PANless_Aadhar(Database); //QUESTION1
        else if(button==2) Print_MULPAN_Aadhar(Database); //QUESTION2
        else if(button==3) Print_MulBank_Aadhar(Database); //QUESTION3
        else if(button==4) Print_LPG_taken_details(Database); //QUESTION4
        else if(button==5){
            float x;
            printf("Enter the threshold amount: "); //QUESTION5
            scanf("%f",&x);
            Print_GreaterThanAmount(Database, x);
        }
        else if(button==6) PrintInconsistent_Data(Database); //QUESTION6
        else if(button==7){
            //NOW FOR MERGING TWO BANK LISTS QUESTION7
            Bank_Data DB1[100];
            Bank_Data  DB2[100];
            int size1=0,size2=0;
            //YOU CAN POPULATE THESE TWO LISTS
            FILE *filePointer4 = fopen("Bank_list1.txt", "r");
            if (filePointer4 != NULL)
            {
                char name[100];
                char address[200];
                char bankname[100];
                int acc,pan;
                float deposit;
                int i=1;
                while (!feof(filePointer4))
                {
                    printf("Taken list1 data: %d\n",i);
                    i++;
                    char buffer[100];
                    fscanf(filePointer4, "%[^\n]s", name);
                    fgets(buffer, sizeof(buffer), filePointer4);
                    fscanf(filePointer4, "%[^\n]s",address);
                    fgets(buffer, sizeof(buffer), filePointer4);
                    fscanf(filePointer4, "%d",&pan);
                    fgets(buffer, sizeof(buffer), filePointer4);
                    fscanf(filePointer4, "%d",&acc);
                    fgets(buffer, sizeof(buffer), filePointer4);
                    fscanf(filePointer4, "%f", &deposit);
                    fgets(buffer, sizeof(buffer), filePointer4);
                    fscanf(filePointer4, "%[^\n]s", bankname);
                    fgets(buffer, sizeof(buffer), filePointer4);
                    fgets(buffer, sizeof(buffer), filePointer4);
                    strcpy(DB1[size1].Name, name);
                    strcpy(DB1[size1].Address, address);
                    strcpy(DB1[size1].Bank_details.BankName, bankname);
                    DB1[size1].Pan_no = pan;
                    DB1[size1].Bank_details.Acc_no = acc;
                    DB1[size1].Bank_details.deposited = deposit;
                    size1++;
                    
                }
                
            fclose(filePointer4);
            }
            else printf("Bank list 1 empty\n");

            FILE *filePointer5 = fopen("Bank_list2.txt", "r");
            if (filePointer5 != NULL)
            {
                char name[100];
                char address[200];
                char bankname[100];
                int acc,pan;
                float deposit;
                int i=1;
                while (!feof(filePointer5))
                {
                    printf("Taken list2 data: %d\n",i);
                    i++;
                    char buffer[100];
                    fscanf(filePointer5, "%[^\n]s", name);
                    fgets(buffer, sizeof(buffer), filePointer5);
                    fscanf(filePointer5, "%[^\n]s",address);
                    fgets(buffer, sizeof(buffer), filePointer5);
                    fscanf(filePointer5, "%d",&pan);
                    fgets(buffer, sizeof(buffer), filePointer5);
                    fscanf(filePointer5, "%d",&acc);
                    fgets(buffer, sizeof(buffer), filePointer5);
                    fscanf(filePointer5, "%f", &deposit);
                    fgets(buffer, sizeof(buffer), filePointer5);
                    fscanf(filePointer4, "%[^\n]s", bankname);
                    fgets(buffer, sizeof(buffer), filePointer4);
                    
                    fgets(buffer, sizeof(buffer), filePointer5);

                    strcpy(DB2[size2].Name, name);
                    strcpy(DB2[size2].Address, address);
                    strcpy(DB2[size2].Bank_details.BankName, bankname);
                    DB2[size2].Pan_no = pan;
                    DB2[size2].Bank_details.Acc_no = acc;
                    DB2[size2].Bank_details.deposited = deposit;
                    size2++;

                }
                
            fclose(filePointer5);
            }
            else printf("Bank list 2 empty\n");



            status_code sc2 = MergeBankList(Database, DB1, size1, DB2, size2); //QUESTION7
            if(sc2== Failure) printf("Merging was a failure\n");
            else printf("Merged Successfully\n");
            
        }
        else if(button==8){
            //Range search
            printf("Enter the Range of Aadhar numbers\n");
            int aadhar1, aadhar2;
            scanf("%d %d", &aadhar1, &aadhar2);
            printf("PRINTING AADHAR DETAILS FROM %d to %d\n",aadhar1, aadhar2);
            RangeSearch(Database, aadhar1, aadhar2);
        }
        
    }






    //Updating the text files
    FILE *foutb, *foutp, *fouta, *foutl;
    foutb = fopen("bankout.txt", "w");
    fouta = fopen("Aadharout.txt", "w");
    foutp = fopen("PANout.txt", "w");
    foutl = fopen("LPGout.txt", "w");
    
    printf("ADDING DATA TO THE FILES\n");
    Bplus_Aadhar *leftmost = FindAadharkeyNode(Database, INT_MIN);
    Aadharleaf *aadharleaves = leftmost->Node.LeafNode;
    while(aadharleaves!=NULL)
    {
        for(int i=0;i<aadharleaves->num_vals;i++){

            fprintf(fouta, "%s\n", aadharleaves->Aadhar_vals[i].Name);
            fprintf(fouta, "%s\n", aadharleaves->Aadhar_vals[i].Address);
            fprintf(fouta, "%d\n\n", aadharleaves->Aadhar_vals[i].Aadhar_no);

            if(aadharleaves->Aadhar_vals[i].LPG_info!=NULL){
                fprintf(foutl, "%s\n", aadharleaves->Aadhar_vals[i].LPG_info->Name);
                fprintf(foutl, "%d\n", aadharleaves->Aadhar_vals[i].LPG_info->acc_no);
                fprintf(foutl, "%d\n\n", aadharleaves->Aadhar_vals[i].LPG_info->taken);
            }
            
            Bplus_PAN *leftpan = FindPANKeyNode(aadharleaves->Aadhar_vals[i].PAN_DB, INT_MIN);
            if(leftpan!=NULL){
                PANleaf *panleaves = leftpan->Node.LeafNode;
                while(panleaves!=NULL)
                {

                    int j=0;
                    while(j<panleaves->num_vals){

                        fprintf(foutp, "%s\n", panleaves->PAN_vals[j].Name);
                        fprintf(foutp, "%s\n", panleaves->PAN_vals[j].Address);
                        fprintf(foutp, "%d\n", panleaves->PAN_vals[j].Aadhar_num);
                        fprintf(foutp, "%d\n\n", panleaves->PAN_vals[j].PAN_no);
                        
                        Bplus_Bank *leftbank = FindBankKeyNode(panleaves->PAN_vals[j].Bank_DB, INT_MIN);
                        if(leftbank!=NULL){
                            Bankleaf *bankleaves = leftbank->Node.LeafNode;
                            while(bankleaves!=NULL){

                                int k=0;
                                while(k<bankleaves->num_vals){
                                    fprintf(foutb, "%s\n", bankleaves->Bank_vals[k].Name);
                                    fprintf(foutb, "%s\n", bankleaves->Bank_vals[k].Address);
                                    fprintf(foutb, "%d\n", bankleaves->Bank_vals[k].Pan_no);
                                    fprintf(foutb, "%d\n", bankleaves->Bank_vals[k].Bank_details.Acc_no);
                                    fprintf(foutb, "%f\n", bankleaves->Bank_vals[k].Bank_details.deposited);
                                    fprintf(foutb, "%s\n\n", bankleaves->Bank_vals[k].Bank_details.BankName);
                                                
                                    k++;
                                }
                                bankleaves = bankleaves->next;
                            }
                            j++;
                        }
                        else j++;
                        
                    }
                    panleaves= panleaves->next;
                }
            }
            printf("---------Updated %s----------\n",aadharleaves->Aadhar_vals[i].Name);
            
        }
        aadharleaves = aadharleaves->next;
        
    }
    fclose(fouta);
    fclose(foutb);
    fclose(foutl);
    fclose(foutp);

    printf("good end\nThank You!\n");




    

}


