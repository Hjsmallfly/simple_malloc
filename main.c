// 原文地址 http://blog.csdn.net/shandongdaya/article/details/7303794

#include <stdio.h>
#include <stdlib.h>
#define HeadSize (sizeof(HeadStruct))

char buf[1000];//模拟内存堆

#define MEM_FREE 0  // 可用块
#define MEM_USED 1  // 已经分配出去的块


typedef struct HeadStruct{
    size_t size; // 块的大小
    void *buf; // 返回给用户使用的起始地址
    struct HeadStruct *pre, *next; // 前驱和后继，用双向链表实现堆
    int used; // 表明此块空闲与否
}HeadStruct; // 堆中块的结构体

void Init_heap(){
    HeadStruct *p = (HeadStruct *)buf;
    p->size = sizeof(buf) - HeadSize;
    p->buf = (char *)p + HeadSize; // p should be cast to char*, 这很重要
    p->pre=p->next = NULL;
    p->used = 0;
}

void list_blocks(){
    HeadStruct *p = (HeadStruct *)buf;
    while(p){
        const char *has_been_used = p->used == 0 ? "free" : "used";
        printf("addr: %-5ld size: %-5lu(%s)\n",(char *)(p->buf)-buf,p->size, has_been_used);
        p=p->next;
    }
}

void *myalloc(size_t size){
    HeadStruct *p=(HeadStruct *)buf;
    // p->size - size > HeadSize 因为 分配一段空间 需要生成一个新的 HeadStruct
    // 整个 while 条件就是找到让 三个条件都满足的块
    while( !( (p != NULL ) && ( p->size - size > HeadSize ) && (p->used == 0 )) ){
        p = p->next;
    }
    if( p == NULL){
        printf("ERROR!");
        return NULL;
    }
    else{
        // rest 是剩下的空闲空间
        HeadStruct *rest= (HeadStruct *) ( (char *)(p->buf) + size);    // size 个字节被分配出去了
        rest->buf = (char *)rest + HeadSize;  // 因为记录 HeadStruct 占去了起始的 HeadSize 个字节
        rest->size = p->size - size - HeadSize; // 余下的空闲空间 记得要减去而外的 HeadSize
        // 双向链表的插入
        rest->next = p->next;
        if(p->next)
          p->next->pre = rest;
        p->next = rest;
        rest->pre = p;
        rest->used = 0; // 可用的区域
        p->size = size;
    }
    p->used = 1;  // p 代表的这块内存已经被分配出去了
    return p->buf;    // 用户可用的实际内存地址
}

void myFree(void *mem){
    // 找到实际的内存快, 因为分配给用户的是实际内存段的后一些部分
    // 所以往前走一个结构体的位置
    HeadStruct *actual = (HeadStruct *)( (char *)mem - HeadSize);
    //堆增长的方式是向高地址增长，要找到块的首地址就的减去块头的大小

    if( !mem || 0 == actual->used){  // 0 表示未使用的
        printf("FREE ERROR!");
        return;
    }
    actual->used = 0; // 修改是否使用的标记
    HeadStruct *tmp;
    if(actual->next && actual->next->used == 0){  // 和后面的部分合并
    actual->size += actual->next->size + HeadSize;
    tmp = actual->next->next;
    actual->next = tmp;
    if(tmp)
      tmp->pre=actual;
    }
    if(actual->pre && actual->pre->used == 0) {   // 和前面部分合并
        actual->pre->size += actual->size + HeadSize;
        actual->pre->next = actual->next;
        if(actual->next)
          actual->next->pre = actual->pre;
    }
    return;
}

void different_pointers(){
    // 演示不同类型指针的差异
    int four_bytes; // 该机器上的整型为4个字节
    int *int_ptr = &four_bytes;
    char *char_ptr = (char *) int_ptr;  // 强制类型转换
    void *void_ptr = (void *) int_ptr;  // 转换成 void *
    char **char_ptr_ptr = &char_ptr;    // 指向 char * 的指针
    printf("sizeof int:  %5lu\n", sizeof(int));
    printf("sizeof char: %5lu\n", sizeof(char));
    printf("sizeof char *: %5lu\n", sizeof(char_ptr));
    printf("sizeof int  *: %5ld\n", sizeof(int_ptr));
    printf("sizeof void *: %5ld\n", sizeof(void_ptr));
    printf("sizeof char **:%5ld\n", sizeof(char_ptr_ptr));
    printf("so all pointers have the same size in a certain machine\n");
    printf("assign 0x41424344 to int (0x41 is the code for 'A')\n");
    four_bytes = 0x41424344;  // ABCD, 不过注意字节序, 即高位存在高位内存还是地位内存
    printf("int  * points to %p\n", int_ptr);
    printf("*(int *): %d\n", *int_ptr);
    printf("char * points to %p\n", char_ptr);
    printf("traverse char *: \n");
    int i;
    for(i = 0 ; i < (int) sizeof(int) ; ++i){
        // %#x 输出带前缀的 hex number
        printf("address: %p integer: %#x ascii: %c\n", char_ptr + i, char_ptr[i], char_ptr[i]);
    }

}

int main(){
    printf("so what exactly is pointer?\n");
    different_pointers();
    printf("---------------------------\n");

    printf("sizeof HeadStruct: %lu\n", sizeof(HeadStruct));
    Init_heap();
    list_blocks();
    printf("---------------allocating %lu bytes---------------\n", sizeof(int) * 100);
    int *a = (int *) myalloc(sizeof(int) * 100);
    list_blocks();
    printf("---------------allocating %lu bytes---------------\n", sizeof(char) * 100);
    char *b = (char *) myalloc(sizeof(char) * 100);
    list_blocks();
    printf("---------------free %lu bytes---------------\n", sizeof(char) * 100 + sizeof(int) * 100);
    myFree(a);
    myFree(b);
    list_blocks();
    return 0;
}
