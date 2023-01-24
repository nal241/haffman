#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "encode.h"

#define NSYMBOLS 256

static int symbol_count[NSYMBOLS];

// 以下このソースで有効なstatic関数のプロトタイプ宣言

// ファイルを読み込み、static配列の値を更新する関数
static void count_symbols(const char *filename);

// symbol_count をリセットする関数
static void reset_count(void);

// 与えられた引数でNode構造体を作成し、そのアドレスを返す関数
static Node *create_node(int symbol, int count, Node *left, Node *right);

// Node構造体へのポインタが並んだ配列から、最小カウントを持つ構造体をポップしてくる関数
// n は 配列の実効的な長さを格納する変数を指している（popするたびに更新される）
static Node *pop_min(int *n, Node *nodep[]);

// ハフマン木を構成する関数
static Node *build_tree(void);


// 以下 static関数の実装
static void count_symbols(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
	fprintf(stderr, "error: cannot open %s\n", filename);
	exit(1);
    }

    // 1Byteずつ読み込み、カウントする
    /*
      write a code for counting
    */
    unsigned char buf;
    while(fread(&buf, sizeof(unsigned char), 1, fp) != 0){
        ++symbol_count[buf];
    }

    fclose(fp);
}
static void reset_count(void)
{
    for (int i = 0 ; i < NSYMBOLS ; i++)
	symbol_count[i] = 0;
}

static Node *create_node(int symbol, int count, Node *left, Node *right)
{
    Node *ret = (Node *)malloc(sizeof(Node));
    *ret = (Node){ .symbol = symbol, .count = count, .left = left, .right = right};
    return ret;
}

static Node *pop_min(int *n, Node *nodep[])
{
    // Find the node with the smallest count
    // カウントが最小のノードを見つけてくる
    int argmin = 0;
    for (int i = 0; i < *n; i++) {
	if (nodep[i]->count < nodep[argmin]->count) {
	    argmin = i;
	}
    }
    
    Node *node_min = nodep[argmin];
    
    // Remove the node pointer from nodep[]
    // 見つかったノード以降の配列を前につめていく
    for (int i = argmin; i < (*n) - 1; i++) {
	nodep[i] = nodep[i + 1];
    }
    // 合計ノード数を一つ減らす
    (*n)--;
    
    return node_min;
}

static Node *build_tree(void)
{
    int n = 0;
    Node *nodep[NSYMBOLS];
    
    for (int i = 0; i < NSYMBOLS; i++) {
	// カウントの存在しなかったシンボルには何もしない
	if (symbol_count[i] == 0) continue;
	
	nodep[n++] = create_node(i, symbol_count[i], NULL, NULL);
	
    }

    const int dummy = -1; // ダミー用のsymbol を用意しておく
    while (n >= 2) {
	Node *node1 = pop_min(&n, nodep);
	Node *node2 = pop_min(&n, nodep);
	
	// Create a new node
	// 選ばれた2つのノードを元に統合ノードを新規作成
	// 作成したノードはnodep にどうすればよいか?
	Node *newnode = create_node(dummy, node1->count + node2->count, node1, node2);
	nodep[n++] = newnode;
    }

    // なぜ以下のコードで木を返したことになるか少し考えてみよう
    return (n==0)?NULL:nodep[0];
}



// Perform depth-first traversal of the tree
// 深さ優先で木を走査する
// 現状は何もしていない（再帰してたどっているだけ）
void traverse_tree(const int depth, const Node *np)
{			  
    static char buf[NSYMBOLS];
    static int n = 0;
    if (np->right == NULL && np->left == NULL){
        if(np->symbol == '\n'){
            printf("\\n:\t");
        }else{
            printf("%c:\t",np->symbol);
        }
        buf[n] = '\0';
        printf("%s\n",buf);
        buf[n] = '-';
        return;
    }
    
    buf[n++] = '0';
    traverse_tree(depth + 1, np->left);
    buf[n - 1] = '1';
    traverse_tree(depth + 1, np->right);
    n--;
}


void traverse_tree2(const Node *np)
{			  
    static char buf[NSYMBOLS];
    static int depth = 0;
    static int max_line = 0;
    static int current_line = 0;
    static int dline[NSYMBOLS];
    static int dline_arg = 0;
    //parent
    if(np->symbol == -1){
        fprintf(stdout, "+-");
    }else{
        if(np->symbol == '\n'){
            fprintf(stdout,"\\n: ");
        }else{
            fprintf(stdout,"%c: ",np->symbol);
        }
        buf[depth] = '\0';
        fprintf(stdout,"%s",buf);
        buf[depth] = '-';
    }
    //last
    if (np->right == NULL || np->left == NULL) return;

    //left
    buf[depth++] = '0';
    traverse_tree2(np->left);
    depth--;

    //right
    buf[depth - 1] = '1';

    max_line += 2;
    dline[dline_arg] = max_line - current_line;
    while(current_line < max_line - 1){
        fprintf(stdout, "\n\e[%dG|", 2 * depth + 1);
        current_line++;
    }
    fprintf(stdout, "\n\e[%dG+-", 2 * depth + 1);
    current_line++;

    depth++;
    dline_arg++;
    traverse_tree2(np->right);
    fprintf(stdout, "\e[%dA", dline[dline_arg]);
    current_line -= dline[dline_arg];
    depth--;
    dline_arg--;
}

// この関数は外部 (main) で使用される (staticがついていない)
Node *encode(const char *filename)
{
    reset_count();
    count_symbols(filename);
    Node *root = build_tree();
     
    if (root == NULL){
	fprintf(stderr,"A tree has not been constructed.\n");
    }

    return root;
}
