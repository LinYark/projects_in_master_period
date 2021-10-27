# sort
'''

//https://www.cnblogs.com/onepixel/articles/7674659.html
//稳定性只有冒泡和插排稳定
//https://blog.csdn.net/zhangfuliang123/article/details/72886675

#include<iostream>
using namespace std;

//1冒排序
//冒泡排序:从左到右，两两大小交换。执行n-1次即可
void maopao(int* a, size_t n)
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n-1; j++) {
            if (a[j] > a[j+1]) {
                swap(a[j], a[j + 1]);
            }
        }
    }
}

//2选择排序
//每次找最小的放到最左边
void xuanze(int* h, size_t len)
{
    for (int i = 0; i < len - 1; ++i)
    {
        int minindex = i;
        for (int j = i + 1; j < len; ++j)
        {
            if (h[j] < h[minindex]) minindex = j;
        }
        swap(h[i], h[minindex]);
    }

}

//3插入排序
//插入排序：从左向右遍历，每次把当前的往前推到放到它该在的位置
void InsertSort(int* a, size_t n)
{
    for (int i = 1; i < n; i++) {
        for (int j = i; j > 0; j--) {
            if (a[j] < a[j-1]) {
                swap(a[j], a[j - 1]);
            }
            else {
                break;
            }

        }
    }

}

//4.希尔排序
//每次分n/2,n/4...组，对每组插排
void ShellSort(int* h, size_t len)
{

    for (int div = len / 2; div >= 1; div /= 2)
        for (int k = 0; k < div; ++k)
            for (int i = div + k; i < len; i += div)
                for (int j = i; j > k; j -= div)
                    if (h[j] < h[j - div]) swap(h[j], h[j - div]);
                    else break;
    return;
}

//5快速排序
//小的放左边，大的放右边，找小的大的，交换。
//！因为边界左边小右边大，所以让右方向查找往前迈一步，故需要先找右边后找左边
//void qs(int* a, int left, int right) {
//    int i = left, j = right, key = a[left];
//    //递归故一定记得边界
//    if (i >= j)return;
//    
//    while (i<j)
//    {
//        while (i < j && a[j] >= key) {
//            j--;
//        }
//        //最终交汇的时候i和j只有可能是相等
//        //！因为最终全部交完完成后，边界左边小右边大，所以应该是使用i-j交汇（相等）之前的i值，如果i想前迈一步，数值就加了个1，如果往前迈一步，数值就直接能拿到。
//        //另外，数值就加了个1再返回去可能会越界，不如直接拿！，所有右j往前迈一步
//        while (i < j && a[i] <= key) {
//            i++;
//        }
//        swap(a[i], a[j]);
//    }
//    swap(a[left], a[i]);
//    qs(a, left, i - 1);
//    qs(a, i + 1, right);
//
//    //跟这个一模一样，因为i=j
//    //swap(a[left], a[j]);
//    //qs(a, left, j - 1);
//    //qs(a, j + 1, right);
//}

void qs(int *a,int l,int r ) {
    int i, j,key;
    i = l; j = r; key = a[l];
    if (i >= j) { return; }
    while (i < j)
    {
        while (i < j && a[j]>=key)
        {
            j--;

        }
        while (i < j && a[i] <= key)
        {
            i++;
        }
        swap(a[i], a[j]);
    }
    swap(a[l], a[j]);
    qs(a, l,j-1);
    qs(a, j + 1,r);
}


//
int main()
{
	int a[] = { 4,7,4 ,2,5,8,3,6,9,2,2};
	qs(a, 0, 10);
	return 0;
}

'''
