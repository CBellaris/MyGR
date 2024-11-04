### new
```c++
int size;
std::cin >> size;
int* arr = new int[size]; // 创建动态长度的数组

// 使用数组
// ...

delete[] arr; // 释放内存
```
### 指针的指针
#### 修改指针本身

如果你需要在函数内部改变一个指针的指向（例如，重新分配内存或改变指针所指向的对象），可以使用双指针。例如：

```c++
cppCopy codevoid changePointer(int** p) {
    *p = new int(20); // 修改指针p的指向
}

int main() {
    int* ptr = nullptr;
    changePointer(&ptr); // ptr现在指向新分配的内存
    delete ptr; // 不要忘记释放内存
    return 0;
}
```

#### 作为函数返回多个值的方式

使用双指针也可以作为函数返回多个值的一种方式，尤其是在需要返回状态或结果的情况下：

```c++
cppCopy codevoid getValues(int** a, int** b) {
    *a = new int(5);
    *b = new int(10);
}

int main() {
    int* x = nullptr;
    int* y = nullptr;
    getValues(&x, &y); // x和y被赋值

    // 使用x和y后要释放内存
    delete x;
    delete y;

    return 0;
}
```

#### 动态内存分配

双指针常用于动态创建二维数组或更高维度的数据结构。例如：

```C++
cppCopy codevoid createArray(int*** arr, int rows, int cols) {
    *arr = new int*[rows];         // 分配行
    for (int i = 0; i < rows; ++i) {
        (*arr)[i] = new int[cols]; // 分配每行的列
    }
}

int main() {
    int** array = nullptr;
    createArray(&array, 3, 4); // 创建一个3x4的二维数组

    // 使用完后要释放内存
    for (int i = 0; i < 3; ++i) {
        delete[] array[i];
    }
    delete[] array;

    return 0;
}
```

#### 接收可变长度字符串
```c++
// 这里的const GLchar *const *string是一个双指针
void glShaderSource(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length)

// shaderSource是一个数组，其中每一个元素都是一个指向字符的指针
const char* shaderSource[] = {
    "#version 330 core\n",
    "void main() {\n",
    "    // Shader code here\n",
    "}\n"
};
// Hint: C++的字符串末尾自动添加'\0'以标示结尾

// 调用 glShaderSource
glShaderSource(shader, 4, shaderSource, nullptr);
```

### typedef

```c++
typedef existing_type new_type_name;

typedef unsigned long ulong;
ulong a = 100;  // 使用别名 ulong 代替 unsigned long

struct Person {
    char name[50];
    int age;
};
typedef struct Person Person;  // 创建结构体的别名
Person p;  // 使用别名创建结构体实例

//函数指针
typedef void (*FuncPtr)(int, double);  // 定义函数指针类型
void myFunction(int a, double b) {
    // 函数实现
}
FuncPtr f = myFunction;  // 使用类型别名

//数组
typedef int IntArray[10];  // 定义一个整型数组的别名
IntArray arr;  // 创建一个包含 10 个整数的数组
```

关于typedef在函数指针的用法：
```c++
#include <iostream>

typedef void (*FuncPtr)(int, double);  // 定义一个函数指针类型

// 定义几个符合签名的函数
void functionA(int a, double b) {
    std::cout << "Function A: " << a << ", " << b << std::endl;
}

void functionB(int a, double b) {
    std::cout << "Function B: " << a * 2 << ", " << b * 2 << std::endl;
}

void functionC(int a, double b) {
    std::cout << "Function C: " << a + 10 << ", " << b + 10 << std::endl;
}

// 主函数
int main() {
    FuncPtr funcArray[] = {functionA, functionB, functionC};  // 使用 typedef 定义的函数指针数组

    for (int i = 0; i < 3; ++i) {
        funcArray[i](5, 3.5);  // 通过函数指针调用每个函数
    }

    return 0;
}
```

### vector

`vector<type> name(10)`新建长度为10的数组 并将所有元素置0

`vector<type> name[10]`新建10个数组 使用`name[index][i]`访问对应元素 或`name[index].begin()`等函数

**erase **对于vector 使用`vec.erase(vec.begin()+i)`删除某一元素

`vector <vector<int>> vec` 创建动态二维数组

`vector.back()` `vector.front()` 获取数组最后(最前)一个元素的引用 不同于 `.begin() .end()` 返回一个迭代器

### quene

1. push() 在队尾插入一个元素
2. pop() 删除队列第一个元素
3. size() 返回队列中元素个数
4. empty() 如果队列空则返回true
5. front() 返回队列中的第一个元素
6. back() 返回队列中最后一个元素

### void*

**通用性**：`void*` 指针可以指向任何类型的数据，使得函数参数或返回值可以是任意类型的指针。这对于编写泛型函数或数据结构（如链表、堆栈等）非常有用。

**类型不确定性**：在一些场合下，数据类型可能在编译时并不确定，使用 `void*` 可以暂时绕过这种限制，直到实际使用时再进行类型转换

**类型转换**：`void*` 指针在使用前必须被显式转换为目标类型的指针。转换时要确保类型匹配，否则可能导致未定义行为

```c
int a = 10;
void* ptr = &a;
int* int_ptr = (int*)ptr;
```

**用`void*` 实现简单的通用编程(常用于c，c++尽量使用模板):**

使用 `void*` 的通用链表节点（C 语言）

```c
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    void* data;
    struct Node* next;
} Node;

Node* createNode(void* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

void printIntNode(Node* node) {
    int* data = (int*)node->data;
    printf("%d\n", *data);
}

int main() {
    int value = 42;
    Node* node = createNode(&value);
    printIntNode(node);
    free(node);
    return 0;
}
```

使用模板的通用链表节点（C++ 语言）

```c++
#include <iostream>

template <typename T>
struct Node {
    T data;
    Node* next;
    Node(T data) : data(data), next(nullptr) {}
};

template <typename T>
void printNode(Node<T>* node) {
    std::cout << node->data << std::endl;
}

int main() {
    Node<int> node(42);
    printNode(&node);
    return 0;
}
```

### 模板特化(Template Specialization)

函数模板的特化：当函数模板需要对某些类型进行特别处理，称为函数模板的特化。例如：

​     template <class T>
1    bool IsEqual(T t1, T t2)
2    {
3         return t1 == t2;
4    }
5
6    int main()
7    {
8         char str1[] = "Hello";
9         char str2[] = "Hello";
10        cout << IsEqual(1, 1) << endl;
11        cout << IsEqual(str1, str2) << endl;  //输出0
12        return 0;
13   }
代码11行比较字符串是否相等。由于对于传入的参数是char *类型的，max函数模板只是简单的比较了传入参数的值，即两个指针是否相等，因此这里打印0。显然，这与我们的初衷不符。因此，max函数模板需要对char *类型进行特别处理，即特化：
1     template <>
2    bool IsEqual(char* t1, char* t2)      //函数模板特化
3    {
4         return strcmp(t1, t2) == 0;
5    }



### extern关键字

#### 1. **外部变量的声明**

当变量在一个文件中定义，但在其他文件中使用时，可以使用`extern`关键字在其他文件中声明该变量。例如：

**文件1 (`file1.c`)：**

```c++
int globalVar = 42;  // 定义变量
```

**文件2 (`file2.c`)：**

```c++
extern int globalVar;  // 声明变量
void someFunction() {
    printf("%d\n", globalVar);  // 使用变量
}
```

#### 2. **链接多个文件**

在大型程序中，`extern`常用于链接多个文件。例如，一个变量在一个文件中定义，在多个其他文件中使用：

**头文件 (`globals.h`)：**

```c++
extern int sharedVar;  // 声明变量
```

**源文件1 (`file1.c`)：**

```c++
#include "globals.h"
int sharedVar = 100;  // 定义变量
```

**源文件2 (`file2.c`)：**

```c++
#include "globals.h"
void someOtherFunction() {
    printf("%d\n", sharedVar);  // 使用变量
}
```

### 类的指针

![image-20220928021414620](C:\Users\CwQ\Documents\笔记\图片\image-20220928021414620.png)

基类指针可以指向派生类对象 但仅访问基类的成员 不能访问派生类中的新成员

基类指针仅访问基类成员和函数 派生类指针可以访问基类和新定义的成员和函数 一般将基类指针转换成派生类指针是不安全的

### static_cast

明确隐式类型转换

static_cast<目标类型>(原数据)

**可以用于低风险的转换**

- 整型和浮点型
- 字符与整形
- 转换运算符
- 空指针转换为任何目标类型的指针

**不可以用与风险较高的转换**

- 不同类型的指针之间互相转换
- 整型和指针之间的互相转换
- 不同类型的引用之间的转换

### 枚举类型enum的命名空间 作用域

在类中使用 类名作为命名空间

class A {

public:

​	enum{ NUM }

}

A::NUM;

### count 与 count_if

```c++
struct student
{
    string name;
    int score;
};
bool compare(student a)
{
    return 90<a.score;
}
int main()
{
    int n;
    cin>>n;
    vector<student> V;
    for(int i=0;i<n;i++)
    {
        student temp;
        cin>>temp.name>>temp.score;
        V.push_back(temp);
    }
    cout<<count_if(V.begin(),V.end(),compare)<<endl;
    return 0;
}
```



### static

在类/结构体外，该变量（函数）仅在本编译单元中被识别，无法通过extern来引用

在函数内，该变量生命周期扩展到整个程序的生命期



### 枚举类
建议都使用枚举类，有较高的类型安全和作用域控制
```c++
enum class Color {
    Red,
    Green,
    Blue
};

int main() {
    Color myColor = Color::Green;  // 需要使用 Color:: 前缀
    int colorValue = static_cast<int>(myColor);  // 显式转换为整数
    return 0;
}
```
也可以显示的指定类型的值：
```c++
enum class Color {
    Red = 1,
    Green = 2,
    Blue = 3
};
```

普通枚举类名不能作为命名空间

### string



### C++与C混编

**解决C不支持函数重载**

(1)函数重载机制是C++的特性，而C语言并没有，根据向前兼容原则，需要C++去兼容C，而不能要求C语言去支持函数重载机制；
(2)解决方法就是C++在需要和C对接的局部不采用函数重载机制，向C兼容；
(3)在C++中，用extern “C”{}括起来的内容表示向C兼容，不要使用函数重载机制；
注意点：extern “C”{}是C++中支持的，在C中是没有extern “C”{}这个用法的，C语言使用编译会报错；

```c++
#ifdef __cplusplus
extern "C"{
#endif

	······

#ifdef __cplusplus
}
#endif
```

### explicit

`explicit`关键字在C++中用于构造函数或转换运算符，主要目的是防止隐式转换。它告诉编译器该构造函数或转换运算符只能通过直接调用来使用，而不能通过隐式类型转换来使用。

考虑以下类定义：

```c++
class Foo {
public:
    Foo(int x) { /* ... */ }
};
```

使用上述类时，可以进行隐式转换：

```c++
void doSomething(Foo f) { /* ... */ }

doSomething(42);  // 隐式转换：int 42 -> Foo(42)
```

为了防止这种隐式转换，可以使用`explicit`关键字：

```c++
class Foo {
public:
    explicit Foo(int x) { /* ... */ }
};
```

现在，隐式转换将被禁止，必须显式调用构造函数：

```c++
doSomething(Foo(42));  // 必须显式调用
```

### 指针函数

```c++
// 返回int指针的函数，下面写法皆可
int *fun(int x,int y);
int * fun(int x,int y)；
int* fun(int x,int y);
```

### 函数指针

```c++
#include <iostream>

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int main() {
    // 声明函数指针
    int (*funcPtr)(int, int);

    // 将函数地址赋值给函数指针
    funcPtr = add;
    std::cout << "Add: " << funcPtr(2, 3) << std::endl; // 输出：Add: 5

    // 将其他函数地址赋值给函数指针
    funcPtr = subtract;
    std::cout << "Subtract: " << funcPtr(5, 3) << std::endl; // 输出：Subtract: 2

    return 0;
}
```

### const

**1. 常量变量**

声明一个常量变量，其值不能被修改：

```c++
const int x = 10;
// x = 20; // 错误：x是常量，不能被修改
```

**2. 常量指针和指向常量的指针**

- 指向常量的指针：指针指向的值不能通过该指针修改，但指针本身可以改变。

  ```c++
  const int a = 10;
  const int b = 20;
  const int *ptr = &a; // ptr指向a
  // *ptr = 20; // 错误：不能修改ptr指向的值
  ptr = &b; // 合法：可以改变ptr的指向
  ```

- 常量指针：指针本身不能改变，但可以通过该指针修改指向的值。

  ```c++
  int a = 10;
  int b = 20;
  int *const ptr = &a; // 常量指针，ptr不能改变
  *ptr = 20; // 合法：可以修改ptr指向的值
  // ptr = &b; // 错误：不能改变ptr的指向
  ```

- 指向常量的常量指针：指针本身和指向的值都不能修改。

  ```c++
  const int a = 10;
  const int *const ptr = &a; // 指向常量的常量指针
  // *ptr = 20; // 错误：不能修改ptr指向的值
  // ptr = &b; // 错误：不能改变ptr的指向
  ```

**3. 常量引用**

常量引用用于避免在传递大型对象时进行拷贝，并防止修改引用的对象：

```c++
void printValue(const int& value) {
    // value不能被修改
    std::cout << value << std::endl;
}

int main() {
    int x = 10;
    printValue(x); // x不会被修改
    return 0;
}
```

**4. 常量成员函数**

在类中，常量成员函数不能修改对象的成员变量。常量成员函数的声明在函数名之后加上 `const` 关键字：

```c++
class MyClass {
public:
    int getValue() const {
        return value;
    }

private:
    int value = 10;
};

int main() {
    MyClass obj;
    std::cout << obj.getValue() << std::endl; // 合法：getValue是常量成员函数
    return 0;
}
```

**5. 函数参数和返回值的常量性**

- 常量函数参数：防止在函数内部修改参数。

  ```c++
  void printValue(const int value) {
      // value不能被修改
      std::cout << value << std::endl;
  }
  ```

- 常量返回值：防止对返回值的修改。

  ```c++
  const int getValue() {
      return 10;
  }
  ```

**6. 常量类成员**

类成员可以是常量，必须在构造函数的初始化列表中初始化：

```c++
class MyClass {
public:
    MyClass(int v) : value(v) {}
    int getValue() const {
        return value;
    }

private:
    const int value; // 常量成员
};

int main() {
    MyClass obj(10);
    std::cout << obj.getValue() << std::endl; // 输出：10
    return 0;
}
```

### static和friend创建类的工具函数

- 使用friend，可访问私有成员

  ```c++
  class MyVector {
  public:
      //成员变量
      double x, y, z;
  	
      // 非成员函数
      friend double Dist(const MyVector&, const MyVector&);
  };
  
  double Dist(const MyVector& v1, const MyVector& v2) {
      return sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
  }
  ```

- 如果你希望在类内部实现一些工具性的功能，并且这些功能不需要访问对象的具体状态，那么可以考虑将它们声明为`static`成员函数

  ```c++
  class MyVector {
  public:
      // 成员变量
      double x, y, z;
  
      // 静态成员函数声明
      static double Dist(const MyVector& v1, const MyVector& v2);
  };
  
  // 静态成员函数定义
  double MyVector::Dist(const MyVector& v1, const MyVector& v2) {
      return sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
  }
  
  // 在其他地方可以直接调用
  double distance = MyVector::Dist(vec1, vec2);
  ```

两种方式定义的函数都是非成员函数，可以在不定义实例的情况下直接使用

## Error

头文件中类成员函数的定义只能放在类体内 不能在类体外定义(多重定义的符号)

与库函数或变量重名(expected unqualified-id)
