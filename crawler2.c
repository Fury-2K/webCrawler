#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include "html.h"

#define HASH_SIZE 100
#define URL_LENGTH 1000
#define SEED_URL "https://www.chitkara.edu.in/"
#define MAX_URL_PER_PAGE 100
//int x=28;                       //because SEED_URL ka keygen == 28

struct base {
	char url[URL_LENGTH];
	int depth;
	int key;
};
struct node {
  struct base b;
	int isVisited;
	struct node *next;
};
struct hash {
	int count;
	int isVisited;
	struct node *head,*tail;
};

void testDir(char b[]) {
  struct stat statbuf;
  if ( stat(b, &statbuf) == -1 )
  {
    fprintf(stderr, "-----------------\n");
    fprintf(stderr, "Invalid directory\n");
    fprintf(stderr, "-----------------\n");
    exit(1);
  }

  //Both check if there's a directory and if it's writable
  if ( !S_ISDIR(statbuf.st_mode) )
  {
    fprintf(stderr, "-----------------------------------------------------\n");
    fprintf(stderr, "Invalid directory entry. Your input isn't a directory\n");
    fprintf(stderr, "-----------------------------------------------------\n");
    exit(1);
  }

  if ( (statbuf.st_mode & S_IWUSR) != S_IWUSR )
  {
    fprintf(stderr, "------------------------------------------\n");
    fprintf(stderr, "Invalid directory entry. It isn't writable\n");
    fprintf(stderr, "------------------------------------------\n");
    exit(1);
  }
  printf("Valid Directory");
}
void testURL(char a[]) {
  int i=0;
  char str1[10000]="wget --spider ";
  strcat(str1,a);
  if(!system(str1)) {
    printf("\nValid URL\n");
  }
  else {
    printf("\nInvalid URL\n");
    exit(1);
  }
}
int getDepth(char** argv) {
	int i=0;
  int num=0;
  for(i=0;argv[3][i]!='\0';i++) {
    num=num*10+argv[3][i]-'0';
  }
	return num;
}
void testDepth(int depth) {
  if(depth<1||depth>5) {
    printf("Invalid depth\n\n");
    exit(1);
  }
  else {
    printf("Valid depth\n\n");
  }
}
void getPage(char a[]) {
  char urlbuffer[URL_LENGTH+300]={0};
  strcat(urlbuffer,"wget -O ");
  strcat(urlbuffer,"storage/temp.txt ");
  strcat(urlbuffer, a);
  system(urlbuffer);
}
void copyFile() {
  FILE *f1,*f2;
  f1=fopen("storage/temp.txt","r");
  f2=fopen("storage/file1.txt","w");
  char ch;
  ch=fgetc(f1);
  while(ch!=EOF) {
    fputc(ch,f2);
    ch=fgetc(f1);
  }
  fclose(f1);
  fclose(f2);
}
int checkURL(char url[],int count,char list[MAX_URL_PER_PAGE][URL_LENGTH]) {
  FILE *result;
	if(count>=MAX_URL_PER_PAGE) {
		//printf("\nHash file FULL!!!!\n");
		return 0;
	}
	if(strstr(url,SEED_URL)==NULL) {
		return 0;
	}
	if(strlen(url)>URL_LENGTH) {
		return 0;
	}
	int j=0;
	while(j!=count) {
		if(strcmp(url,list[j])==0){
			return 0;
		}
		j++;
	}
	return 1;
}
int keyGen(char url[]) {
	int sum=0,i=0;
	while(url[i]!='\0') {
		sum+=url[i];
		i++;
	}
	return sum%100;
}
void printList(struct node *start) {
	struct node *ptr;
	ptr=start;
	//FILE *result;
	int j=1;
	//result=fopen("storage/result.txt","a");
	printf("SNo\tDep\tisV\tKey\tURL\n\n");
	while(ptr!=NULL) {
		printf("%d\t%d\t%d\t%d\t%s\n",j,ptr->b.depth,ptr->isVisited,ptr->b.key,ptr->b.url);
		//fprintf(result,"%d\t%s\n",j,ptr->b.url);
		j++;
		ptr=ptr->next;
	}
	//fclose(result);
}

int GetNextURL(char* html, char* urlofthispage, char* result, int pos) {
  char c;
  int len, i, j;
  char* p1;  //!< pointer pointed to the start of a new-founded URL.
  char* p2;  //!< pointer pointed to the end of a new-founded URL.

  // NEW
  // Clean up \n chars
  if(pos == 0) {
    removeWhiteSpace(html);
  }
  // /NEW

  // Find the <a> <A> HTML tag.
  while (0 != (c = html[pos]))
  {
    if ((c=='<') &&
        ((html[pos+1] == 'a') || (html[pos+1] == 'A'))) {
      break;
    }
    pos++;
  }
  //! Find the URL it the HTML tag. They usually look like <a href="www.abc.com">
  //! We try to find the quote mark in order to find the URL inside the quote mark.
  if (c)
  {
    // check for equals first... some HTML tags don't have quotes...or use single quotes instead
    p1 = strchr(&(html[pos+1]), '=');

    if ((!p1) || (*(p1-1) == 'e') || ((p1 - html - pos) > 10))
    {
      // keep going...
      return GetNextURL(html,urlofthispage,result,pos+1);
    }
    if (*(p1+1) == '\"' || *(p1+1) == '\'')
      p1++;

    p1++;

    p2 = strpbrk(p1, "\'\">");
    if (!p2)
    {
      // keep going...
      return GetNextURL(html,urlofthispage,result,pos+1);
    }
    if (*p1 == '#')
    { // Why bother returning anything here....recursively keep going...

      return GetNextURL(html,urlofthispage,result,pos+1);
    }
    if (!strncmp(p1, "mailto:",7))
      return GetNextURL(html, urlofthispage, result, pos+1);
    if (!strncmp(p1, "http", 4) || !strncmp(p1, "HTTP", 4))
    {
      //! Nice! The URL we found is in absolute path.
      strncpy(result, p1, (p2-p1));
      return  (int)(p2 - html + 1);
    } else {
      //! We find a URL. HTML is a terrible standard. So there are many ways to present a URL.
      if (p1[0] == '.') {
        //! Some URLs are like <a href="../../../a.txt"> I cannot handle this.
	// again...probably good to recursively keep going..
	// NEW

        return GetNextURL(html,urlofthispage,result,pos+1);
	// /NEW
      }
      if (p1[0] == '/') {
        //! this means the URL is the absolute path
        for (i = 7; i < strlen(urlofthispage); i++)
          if (urlofthispage[i] == '/')
            break;
        strcpy(result, urlofthispage);
        result[i] = 0;
        strncat(result, p1, (p2 - p1));
        return (int)(p2 - html + 1);
      } else {
        //! the URL is a absolute path.
        len = strlen(urlofthispage);
        for (i = (len - 1); i >= 0; i--)
          if (urlofthispage[i] == '/')
            break;
        for (j = (len - 1); j >= 0; j--)
          if (urlofthispage[j] == '.')
              break;
        if (i == (len -1)) {
          //! urlofthis page is like http://www.abc.com/
            strcpy(result, urlofthispage);
            result[i + 1] = 0;
            strncat(result, p1, p2 - p1);
            return (int)(p2 - html + 1);
        }
        if ((i <= 6)||(i > j)) {
          //! urlofthis page is like http://www.abc.com/~xyz
          //! or http://www.abc.com
          strcpy(result, urlofthispage);
          result[len] = '/';
          strncat(result, p1, p2 - p1);
          return (int)(p2 - html + 1);
        }
        strcpy(result, urlofthispage);
        result[i + 1] = 0;
        strncat(result, p1, p2 - p1);
        return (int)(p2 - html + 1);
      }
    }
  }
  return -1;
}
void NormalizeWord(char* word) {
  int i = 0;
  while (word[i]) {
      // NEW
    if (word[i] < 91 && word[i] > 64) // Bounded below so this funct. can run on all urls
      // /NEW
      word[i] += 32;
    i++;
  }
}
int NormalizeURL(char* URL) {
  int len = strlen(URL);
  if (len <= 1 )
    return 0;
  //! Normalize all URLs.
  if (URL[len - 1] == '/')
  {
    URL[len - 1] = 0;
    len--;
  }
  int i, j;
  len = strlen(URL);
  //! Safe check.
  if (len < 2)
    return 0;
  //! Locate the URL's suffix.
  for (i = len - 1; i >= 0; i--)
    if (URL[i] =='.')
      break;
  for (j = len - 1; j >= 0; j--)
    if (URL[j] =='/')
      break;
  //! We ignore other file types.
  //! So if a URL link is to a file that are not in the file type of the following
  //! one of four, then we will discard this URL, and it will not be in the URL list.
  if ((j >= 7) && (i > j) && ((i + 2) < len))
  {
    if ((!strncmp((URL + i), ".htm", 4))
        ||(!strncmp((URL + i), ".HTM", 4))
        ||(!strncmp((URL + i), ".php", 4))
        ||(!strncmp((URL + i), ".jsp", 4))
        )
    {
      len = len; // do nothing.
    }
    else
    {
      return 0; // bad type
    }
  }
  return 1;
}
void removeWhiteSpace(char* html) {
  int i;
  char *buffer = malloc(strlen(html)+1), *p=malloc (sizeof(char)+1);
  memset(buffer,0,strlen(html)+1);
  for (i=0;html[i];i++)
  {
    if(html[i]>32)
    {
      sprintf(p,"%c",html[i]);
      strcat(buffer,p);
    }
  }
  strcpy(html,buffer);
  free(buffer); free(p);
}

struct node *createHash(char url[],struct hash hash[HASH_SIZE],struct node *start,int depth) {
	struct node *ptr2,*ptr1,*n;
	int val = keyGen(url);
	n=(struct node*)calloc(1,sizeof(struct node));
  int j=0;
  strcpy(n->b.url,url);
	n->b.depth=depth;
	n->b.key=val;
	n->isVisited=0;
	//printf("\n%d\t%d\t%s",val,hash[val-1].count,n->b.url);
	if(start==NULL) {
		start=n;
		n->b.depth=0;
		//current=start;
		n->isVisited=1;
		n->next=NULL;
		hash[val].head=n;
		hash[val].tail=n;
		hash[val].count+=1;
		//printf("\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	}
	else {
		if(hash[val].head==NULL) {
			hash[val].head=n;
			hash[val].tail=n;
			hash[val].count+=1;
			ptr1=start;
			while(ptr1->next!=NULL) {
				ptr1=ptr1->next;
			}
			ptr1->next=n;
			n->next=NULL;
		}
		else {
			//printf("\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n%s\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n",n->b.url);
			struct node *temp1;
			temp1=hash[val].head;
			int t=1;
			int f=0;
			t=1;
			if(hash[val].count==1) {
				if(strcmp(hash[val].head->b.url,n->b.url)==0) {
					//printf("\n77777777777777777777777777777777777777777777777777\n");
					f=1;
				}
			}
			else {
				temp1=hash[val].head;
				while(t<hash[val].count) {
					if(strcmp(temp1->b.url,n->b.url)==0) {
						//printf("\n77777777777777777777777777777777777777777777777777\n");
						f=1;
						break;
					}
					else {
						temp1=temp1->next;
						t++;
					}
				}
			}
			// printf("\n---------------------------------------------------------------------\n");
			// while(t<hash[val].count) {
			// 	printf("%s\n",temp1->b.url);
			// 	temp1=temp1->next;
			// 	t++;
			// }
			// printf("\n---------------------------------------------------------------------\n");
			if(f!=1) {
				// n->next=temp1->next;
				// temp1->next=n;
				// hash[val].tail=n;
				// hash[val].count++;
				struct node *last;
				last=hash[val].tail;
				ptr1=last;
				//printf("\n12112121212\n%s\n",ptr1->b.url);
				if(ptr1->next!=NULL) {
					ptr1=ptr1->next;
					//printf("----");
					last->next=n;
					n->next=ptr1;
				}
				else {
					ptr1->next=n;
					n->next=NULL;
				}
				hash[val].tail=n;
				hash[val].count+=1;
				struct node *temp;
				int flag=0;
				temp=hash[val].head;
				while(temp!=NULL) {
					if(temp->isVisited==0) {
						flag=0;
						break;
					}
					else {
						flag=1;
						temp=temp->next;
					}
				}
				if(flag==1) {
					hash[val].isVisited=1;
				}
			}
	  }
	//printf("\ngg=%d\n",hash[val-1].count);
	}
	// printf("\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	// struct node *temp=start;
	// while(temp!=NULL) {
	// 	printf("%s\n",temp->b.url);
	// 	temp=temp->next;
	// }
	// printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	return start;
}
struct node *crawl(int depth,struct hash hash[HASH_SIZE],struct node *start,char u[]) {
	int i=0,posi=0;
	FILE *fp;
	fp=fopen("storage/temp.txt","r");
	char url[URL_LENGTH];
	url[0]='\0';
	char list[MAX_URL_PER_PAGE][URL_LENGTH];
	char str[999999],ch;
	int m=0,count=1,index=0;
	ch=getc(fp);
	while(ch!=EOF) {
		str[m]=ch;
		m++;
		ch=getc(fp);
	}
	str[m]='\0';
	while(posi!=-1) {
		if(url[0]!='\0') {
			if(checkURL(url,count,list)==1&&strcmp(url,SEED_URL)!=0) {
				//printf("\n\n::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n%s\n::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n",url);
				strcpy(list[index],url);
				//printf("%d\t%s\n",count,url);
				//start=createHash(url,hash,start,depth);
				index++;
				count++;
			}
			//url[0]='\0';
			memset(url,0,sizeof(url));
		}
		posi=GetNextURL(str,u,url,posi);
	}
	// printf("\n\n[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]\n");
	// for(i=0;i<index;i++) {
	// 	printf("%s\n",list[i]);
	// }
	// printf("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]\n");
	for(i=0;i<index;i++) {
		start=createHash(list[i],hash,start,depth);
	}
	fclose(fp);
	//printf("\n Total unique URLs found = %d \n",count);
	return start;
}

int main(int argc,char** argv) {
  int i,dep,dep2;
	struct hash hash[100];
	struct node *start,*ptr;
	start=NULL;

  char a[1000],b[1000];
  i=0;
  while(argv[1][i]!='\0') {
		a[i]=argv[1][i];
		i++;
	}
	i=0;
	while(argv[2][i]!='\0') {
		b[i]=argv[2][i];
		i++;
	}
	testDir(b);
  testURL(a);
	dep=getDepth(argv);
  testDepth(dep);
	getPage(a);
  copyFile();
  for(i=0;i<100;i++) {
		hash[i].count=0;
		hash[i].isVisited=0;
		hash[i].head=NULL;
		hash[i].tail=NULL;
	}
	dep2++;
	printf("\n////////////////////////////////////////////////////////////\n");
	printf("1\t%s",SEED_URL);
	printf("\n////////////////////////////////////////////////////////////\n\n");
	start=createHash(SEED_URL,hash,start,dep2);
	start=crawl(dep2,hash,start,a);
  printList(start);

	ptr=start;
  while(ptr!=NULL) {
		if(ptr->isVisited!=1) {
			if(ptr->b.depth<dep) {
				getPage(ptr->b.url);
				ptr->isVisited=1;
				//start=createHash(ptr->b.url,hash,start,dep);
				start=crawl(dep,hash,start,ptr->b.url);
				printf("\n////////////////////////////////////////////////////////////\n");
				printf("%d\t%s",ptr->b.depth,ptr->b.url);
				printf("\n////////////////////////////////////////////////////////////\n\n");
				//printList(start);
			}
		}
		ptr=ptr->next;
	}
  printf("\n\n::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	printList(start);
	printf("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
}
