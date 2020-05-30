#include <iostream>
#include <ctime>				// ʹ��clock������ʱ
#include "Point.h"
#include <cmath>
#include "Strategy.h"
#include "Judge.h"
//#include <conio.h>				// ������Դ�ӡ
//#include <atlstr.h>
#define MAX_N 12
#define upper_Used 999980
#define upper_Case 1000000
using namespace std;
#define FLT_MAX 100000

struct Node {
    Node* children[MAX_N];
    Node* parent;
    float score;  //���ڼ��㣬��ֻ��ȡ����
    float sum_times;
};

Node tree[upper_Case];

enum Board {
    empty=0,
    me=2,
    you=1,
    no=3
};

/*
	���Ժ����ӿ�,�ú������Կ�ƽ̨����,ÿ�δ��뵱ǰ״̬,Ҫ�����������ӵ�,�����ӵ������һ��������Ϸ��������ӵ�,��Ȼ�Կ�ƽ̨��ֱ����Ϊ��ĳ�������

	input:
		Ϊ�˷�ֹ�ԶԿ�ƽ̨ά����������ɸ��ģ����д���Ĳ�����Ϊconst����
		M, N : ���̴�С M - ���� N - ���� ����0��ʼ�ƣ� ���Ͻ�Ϊ����ԭ�㣬����x��ǣ�����y���
		top : ��ǰ����ÿһ���ж���ʵ��λ��. e.g. ��i��Ϊ��,��_top[i] == M, ��i������,��_top[i] == 0
		_board : ���̵�һά�����ʾ, Ϊ�˷���ʹ�ã��ڸú����տ�ʼ���������Ѿ�����ת��Ϊ�˶�ά����board
				��ֻ��ֱ��ʹ��board���ɣ����Ͻ�Ϊ����ԭ�㣬�����[0][0]��ʼ��(����[1][1])
				board[x][y]��ʾ��x�С���y�еĵ�(��0��ʼ��)
				board[x][y] == 0/1/2 �ֱ��Ӧ(x,y)�� ������/���û�����/�г������,�������ӵ㴦��ֵҲΪ0
		lastX, lastY : �Է���һ�����ӵ�λ��, ����ܲ���Ҫ�ò�����Ҳ������Ҫ�Ĳ������ǶԷ�һ����
				����λ�ã���ʱ��������Լ��ĳ����м�¼�Է������ಽ������λ�ã�����ȫȡ�������Լ��Ĳ���
		noX, noY : �����ϵĲ������ӵ�(ע:��ʵ���������top�Ѿ����㴦���˲������ӵ㣬Ҳ����˵���ĳһ��
				������ӵ�����ǡ�ǲ������ӵ㣬��ôUI�����еĴ�����Ѿ������е�topֵ�ֽ�����һ�μ�һ������
				��������Ĵ�����Ҳ���Ը�����ʹ��noX��noY��������������ȫ��Ϊtop������ǵ�ǰÿ�еĶ�������,
				��Ȼ�������ʹ��lastX,lastY�������п��ܾ�Ҫͬʱ����noX��noY��)
		���ϲ���ʵ���ϰ����˵�ǰ״̬(M N _top _board)�Լ���ʷ��Ϣ(lastX lastY),��Ҫ���ľ�������Щ��Ϣ�¸������������ǵ����ӵ�
	output:
		������ӵ�Point
*/

class UTC {
public:
//private:
    Point* p;
    int M;  //��������
    int N;  //��������
    int** board;  //�����в��������
    int* top;  //Ҳ����
    int** board_;  //�仯��
    int* top_;  //�仯��
    int last_x;  //ֻ��Ϊ�˲�����������һ��
    int last_y;
    int noX;  //���ŵ�
    int noY;
    int cur_player;  //ֻ��Ϊ�˲�������
//
    int used_num;  //����ѭ������Ϊ�����ж�
    Node* root;
    enum Player {
        me_=2, //Ҳ�������Լ���û�취����������
        you_=1 //��������һ��
    };
    enum Board {
        empty=0,
        me=2,
        you=1,
        no=3
    };
    enum Result {
        win_loss = 1,
        tie = 2,
        not_end = 3,
    };
public:
    Node* newNode(Node* parent) {
        auto new_node = &tree[used_num++];
        new_node->parent=parent;
        new_node->score=0;
        new_node->sum_times=0;
        memset(new_node->children, 0, sizeof(new_node->children));
        return new_node;
    }
    //���캯��
    UTC(int M, int N, const int* _top, const int* _board,
        int noX, int noY):M(M),N(N),noX(noX),noY(noY),used_num(0),last_x(-1),last_y(-1) {  //����getPoint�Ĳ���
        root=newNode(nullptr);
        board = new int*[M];
        board_ = new int*[M];
        for(int i = 0; i < M; i++) {
            board[i] = new int[N];
            board_[i] = new int[N];
        }
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                board[i][j]=_board[i*N+j];
            }
        }
        board[noX][noY]=Board::no;
        top = new int[N];
        top_ = new int[N];
        copy(_top,_top+N, top);
    }
    ~UTC() {
        delete[] top;
        delete[] top_;
        for(int i = 0; i < M; i++) {
            delete[] board[i];
            delete[] board_[i];
        }
        delete[] board;
        delete[] board_;
    }
    //������Ҫ����һ����Ԫ��
    Point* utcRoutine(float time_limit = 2.9) {  //���ֻ����һ�Σ���֤���ĵ������ڹ���֮��һ������
        clock_t start_time = clock();
        srand(time(0));
        while((((clock()-start_time)/(float)CLOCKS_PER_SEC)<time_limit)&&used_num<upper_Used) {
            resetBoard();
            auto node = treePolicy();
            float score=defautPolicy();  //��Ȼ��ʵ������������Ϊ�˲�����������ʧ������ת��
            backUp(node, score);
        }
        //printf("%d\n", used_num);
        int y=decideSolution();
        int x=board[top[y]-1][y]==Board ::empty?top[y]-1:top[y]-2;
        p = new Point(x,y);  //
        return p;
    }
    float defautPolicy() {
        const int ini_player = cur_player;
        int count = 0;
        int feasible_num;
        bool move[MAX_N]={0};
        int feasible[MAX_N];
        while(true) {
            count++;
            switch(Judge(last_x, last_y)) {
                case Result::win_loss: {
                    int score = count == 1 ? 5 : 1;
                    return cur_player == ini_player ? score : -score;
                };
                case Result::tie:return 0;
            }
            feasible_num=0;
            //��������ʵ��û�б�Ҫ��ʼ������Ϊ����϶������¸�ֵ
            for(int i = 0; i < N; i++) {
                if((move[i]=(top_[i]>0&&board_[top_[i]-1][i]==Board::empty))||top_[i]>1) {
                    feasible[feasible_num]=i;
                    feasible_num++;
                }
            }
            int chosed = feasible[rand()%feasible_num];
            cur_player=3-cur_player;
            last_y=chosed;
            last_x=move[last_y]?--top_[last_y]:--(--top_[last_y]);
            board_[last_x][last_y]=cur_player;
        }
    }
    void backUp(Node* node, float score) {
        while(node) {
            node->sum_times++;
            node->score+=score;
            score=score>1?1-score:(score<-1?-score-1:-score);
            node=node->parent;
        }
    }
    int decideSolution() {
        float max_value = -FLT_MAX;
        int chosed = -1;
        float tmp_value;
        Node* tmp_child;
        for(int i = 0; i < N; i++) {
            if((tmp_child = root->children[i])) {
                tmp_value=tmp_child->score/tmp_child->sum_times;
                if(tmp_value>max_value) {
                    chosed=i;
                    max_value=tmp_value;
                }
            }
        }
        return chosed;
    }
    int Judge(int lastX,int lastY) {  //Ϊʲô��Ҫ���ܲ���������Ϊģ�⣨��֦����Ҫ�޸�last_x�������4�仰
//        1��ʤ���Ѷ���0��δ����2��ƽ��
        if(cur_player==Player::me_ && machineWin(lastX,lastY, M, N, board_)) {
            return Result::win_loss;
        }
        if(cur_player==Player::you_ && userWin(lastX,lastY, M, N, board_)) {
            return Result::win_loss;
        }
        for(int i = 0; i < N; i++) {
            if(board_[0][i]==Board::empty) return Result::not_end;
        }
        return Result::tie;
    }
    Node* treePolicy() {
        Node* root_ = root;
        cur_player=Player::me_;  //��Ϊȫ�ֱ�������ʱδ��
        int feasible_num;
        int feasible[MAX_N];
        int tmp_x;
         //����Ǹ���ʱ��
        int chosed = -1;
        //��feasible���ף�����Ϊ�˼��top_�費��Ҫ��һ�������ƶ�����ʼ��ʼ��Ϊ0�������1���Ǿ��ǲ���Ҫ�ƶ�
        while(true) {
            bool win_flag = false;
            feasible_num=0;
            for(int i = 0; i < N; i++) {
                if((top_[i]>0&&board_[top_[i]-1][i]==Board::empty)||top_[i]>1) {
                    feasible[feasible_num]=i;
                    feasible_num++;
                    //���ڿ����ǲ�����ô�߾ͻ����
                    tmp_x = board_[top_[i]-1][i]==Board::empty?top_[i]-1:top_[i]-2;
                    board_[tmp_x][i]=cur_player; //�����Ҿ����һ���ط�
                    if(Judge(tmp_x,i)==Result::win_loss) {
                        win_flag=true;
                        feasible_num = 1;
                        feasible[0]=i;
                        board_[tmp_x][i]=Board::empty;
                        break;
                    }
                    board_[tmp_x][i]=Board::empty;
                }
            }
            if(feasible_num==0) return root_;
            Node* tmp_node = expand(root_, feasible, feasible_num, chosed);
            if(chosed<0) chosed=selectChild(root_,feasible, feasible_num);
            last_y=chosed;
            last_x=board_[top_[last_y]-1][last_y]==Board::no?--(--top_[last_y]):--top_[last_y];
            board_[last_x][last_y]=cur_player;
            if(tmp_node) return tmp_node;
            if(win_flag) return root_->children[feasible[0]];  //��ʤ���
            root_=root_->children[chosed];
            cur_player=3-cur_player;
        }
    }
    Node* expand(Node* root_, const int* feasible, const int feasible_num, int& chosed) {
        for(int i = 0; i < feasible_num;i++) {
            if(!root_->children[feasible[i]]) {
                chosed=feasible[i];
                root_->children[chosed]=newNode(root_);
                return root_->children[chosed];
            }
        }
        chosed=-1;
        return nullptr;
    }
    void resetBoard() {
        //������Ǹ���һ�����̺�top����board_
        cur_player = 2;
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++){
                board_[i][j] = board[i][j];
            }
        }
        copy(top, top+N, top_);
    }
    int selectChild(Node* root_, const int* feasible, const int feasible_num, int C=1) { //C��ʾ��ʽ�еĲ���
        float max_value = -FLT_MAX;
        float tmp_value;
        int chosed = -1;
        Node* tmp_child;
        float log_sum = C*2*log(root_->sum_times);
        for(int i = 0; i < feasible_num; i++) {
            tmp_child = root_->children[feasible[i]];
            tmp_value = tmp_child->score/tmp_child->sum_times+sqrt(log_sum/tmp_child->sum_times);
                if(tmp_value>max_value) {
                    chosed=feasible[i];
                    max_value=tmp_value;
                }
        }
        return chosed;
    }
//    void printBoard_(){
//        printf("\nboard_\n");
//        for(int i = 0; i < M; i++) {
//            for(int j = 0; j < N; j++) {
//                printf("%d ", board_[i][j]);
//            }
//            printf("\n");
//        }
//        for(int j = 0; j < N; j++) {
//            printf("%d ", top_[j]);
//        }
//        printf("\n==================\n");
//
//    }
//    void printBoard(){
//        printf("\nboard\n");
//        for(int i = 0; i < M; i++) {
//            for(int j = 0; j < N; j++) {
//                printf("%d ", board[i][j]);
//            }
//            printf("\n");
//        }
//        for(int j = 0; j < N; j++) {
//            printf("%d ", top[j]);
//        }
//        printf("\n==================\n");
//
//    }
};

Point* getPoint(const int M, const int N, const int* top, const int* _board,
                const int lastX, const int lastY, const int noX, const int noY) {
    UTC utc(M,N,top,_board,noX,noY);
    auto a= utc.utcRoutine();
    return a;
}

/*
getPoint�������ص�Pointָ�����ڱ�dllģ���������ģ�Ϊ��������Ѵ���Ӧ���ⲿ���ñ�dll�е�
�������ͷſռ䣬����Ӧ�����ⲿֱ��delete
*/
void clearPoint(Point* p) {  //��ʱ�ᱻ��s�ã�
    delete p;
    return;
}


void clearArray(int M, int N, int** board) { //����delete board
    for(int i = 0; i < M; i++){
        delete[] board[i];
    }
    delete[] board;
}



