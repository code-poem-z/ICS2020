#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "cpu/reg.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

int init=0; // 记录已被使用的监视点数量

void init_wp_pool();

WP* new_wp(){ //启用一个空闲的监视点
   if(init==0){
     init_wp_pool();
   }
   WP* p=free_;
   if(p){  //还有没有被使用的监视点结构
     if(free_->next==head)  //置空free_,即监视点已被全部使用
        free_=NULL;
      else
       free_=free_->next;
     p->next=head;
     head=p;
     init++;//被使用的监视点+1
     return p;
   }
   else{
     printf("监视点已被全部使用！\n");
     assert(0);
   }

}

int free_wp(int NO){  //释放一个空闲的监视点
    WP* p=head;
    WP* pre=head;
    if(head==NULL){
      printf("No watchpoint！\n");
      return 0;
    }
    else if(head->NO==NO){ //头结点就是这个监视点
      if(head->next==free_){
        head=NULL;   //置空head,即监视点全部空闲
      }
      else{
        head=head->next;
      }
      p->next=free_;   //头插法
      free_=p;
      init--;  // 被使用的监视点数-1
      printf("Watchpoint %d has been deleted！ \n",NO);
      return 0;
    }
    else{
       while(p->NO!=NO&&p->next){
         pre=p;
         p=p->next;
       }
       if(p->NO==NO){
         pre->next=p->next;
         p->next=free_;
         free_=p;
         init--; // 被使用的监视点数-1
         printf("Watchpoint %d has been deleted！ \n",NO);
       }
       else{
         printf("No  watchpoint %d\n!",NO);
       }
    }
    return 0;
}

int set_watchpoint(char *e) {
  uint32_t val;
  bool success;
  val = expr(e, &success);
  if(!success) return -1;
  WP* p;
  p=new_wp();        //启用监视点
  strcpy(p->exprs,e);
  p->old_val=val;   // 赋旧值
  printf("Set watchpoint %d !\n", p->NO);
  printf("expr = %s\nold  value = %#x\n",e,val);
  return p->NO;     // 返回监视点编号
}

bool delete_watchpoint(int NO){
   free_wp(NO);
   return true;
}

typedef struct point{
  int no;
  char exprssion[32];
  int Value;
}wa; //定义结构体 储存监视点信息


void list_watchpoint(){
  WP* p=head;
  if(!p){
    printf("No watchpoint now!\n");
    return;
  }
  wa watp[32]; 
  int i=0;
  while(p){
     watp[i].no=p->NO;  // 由于更新监视点信息采用头插法，导致了遍历过程中，编号大的先输出,所以定义数组先储存再倒叙输出
     strcpy(watp[i].exprssion,p->exprs);
     watp[i++].Value=p->old_val;
     p=p->next;
  }
  printf("N0    Expr        old value\n");
  for(int j=i-1;j>=0;j--){
    printf("%d     %s        %#x\n",watp[j].no,watp[j].exprssion,watp[j].Value);
  }
  return;
}  

WP* scan_watchpoint(void){
   WP* p=head;
   if(!p){
    printf("No watchpoint now\n!");
    return false;
   }
   else{
     bool success;
     p->new_val=expr(p->exprs,&success);
     if(!success){
       printf("Fail to eval the  new_val of watchpoint %d (%s) !",p->NO,p->exprs);
       return false;
     }
     else{
       if(p->new_val!=p->old_val){  // 新旧值不同，监视点被触发
          printf("Hit watchpoint %d at address %#010x \n",p->NO,cpu.eip);
          printf("expr      = %s\n",p->exprs);
          printf("Old value = %#x\n",p->old_val);
          printf("New value = %#x\n",p->new_val);
          p->old_val=p->new_val;   // 更新旧值
          printf("program paused\n");
       }
     }
   }
   return p;

}

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */


