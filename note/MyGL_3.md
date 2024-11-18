## NDC (Normalized Device Coordinate)
整个OpenGL对标准坐标的定义和转换流程在这里忽略，仅从代码实现的角度做一遍梳理：

我们不从坐标系转换的角度去看整个流程，而是从更符合直觉的角度去思考，即不移动坐标系，转而移动模型，即所有这些矩阵都是在移动模型，坐标系从始至终只有世界坐标系

可以想象一个具体的坐标变换流程，首先模型的所有顶点都以坐标原点为中心定义，然后我们定义模型的缩放、旋转和平移参数，由这些参数构成的模型变换矩阵(model matrix)，将模型从原点移动到我们想让其在的地方

接着定义摄像机的位置和朝向(外属性)，由这些属性创建的摄像机移动矩阵(view matrix)，可以将摄像机从原点移动到我们想让它在的地方，但是程序最后会从OpenGL的默认摄像机位置和方向渲染物体(即位置(0,0,1)，朝向-z，上向量为+y)，那我们反向去使用这个矩阵，将模型变换回去(想象模型和摄像机粘在一起了，我们捏着摄像机，给它拖回到原点)，那么从默认摄像机渲染的视角，就等同于从我们定义的摄像机渲染的视角

最后是摄像机的画幅和FOV(内属性)，这里的透视投影(projection matrix)就不涉及坐标转换了，很好理解

如果你对坐标系的转换非常熟悉，上面的理解流程可能会有些冗余，那这时只要记住，对于两个坐标系A和B，在A系表示的B系的三个基构成的矩阵R_B，对于在B系表示的向量V_B，我们可以得到在A系表示的此向量V_A = R_B * V_B (仅对旋转)。那么对于世界坐标系W，模型坐标系M，摄像机坐标系C，我们首先明确，R_M和R_C是在W系的表示，那么对于M系的向量V_M，首先将其转换到系W: V_W = R_M * V_M，然后我们要做的是将其转换至系C，这是一个反向的过程，即: V_C = R_C' * V_W，这里的R_C'是R_C的转置

然后我们就能对整个坐标系转换在程序中的流程做一下梳理，首先需要一个模型类来记录模型的缩放、旋转和平移参数，然后提供一些接口方便的调节它们，并且返回计算的模型变换矩阵。接着是一个摄像机类，存储位置，朝向，画幅和FOV等，同样用接口返回view matrix和projection matrix。最后我们考虑在主程序中相乘这些变换矩阵，并传入shader与顶点相乘，在这里没必要将这些矩阵分别传入shader相乘，因为对于我们这个简单渲染器，这些矩阵的改变不会很频繁
### Mesh
我们首先创建一个基础的网格类Mesh，

### 四元数


## 回调函数的设计
如果我们想使用键盘回调函数，用于处理摄像机的移动：
```cpp
Camera* camera; 
float deltaTime;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    bool Press_W = (key == GLFW_KEY_W && action == GLFW_PRESS);
    bool Press_A = (key == GLFW_KEY_A && action == GLFW_PRESS);
    bool Press_S = (key == GLFW_KEY_S && action == GLFW_PRESS);
    bool Press_D = (key == GLFW_KEY_D && action == GLFW_PRESS);

    camera->processKey(Press_W, Press_A, Press_S, Press_D, deltaTime);
}

int main() {
    // 初始化 camera 和 deltaTime
    camera = new Camera();
    deltaTime = 0.0f;

    // 初始化 GLFW
    // ...

    glfwSetKeyCallback(window, key_callback);

    // 主循环
    // ...
}
```
由于我们需要在回调函数中使用camera类和deltaTime，所以一个最简单的方法是直接将其定义为全局变量，就如同上面的代码所做的，但这显然只适用与非常小型的项目。glfw提供了**用户指针**，可以解决这个问题，你可以使用 glfwSetWindowUserPointer() 函数为窗口对象设置一个用户指针，然后在回调函数中通过 glfwGetWindowUserPointer() 来获取它，将 camera 和其他需要的数据封装到一个结构体中，然后通过用户指针将这些数据传递给回调函数：
```cpp
struct AppData {
    Camera* camera;
    float* deltaTime;
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    // 获取用户指针
    AppData* appData = static_cast<AppData*>(glfwGetWindowUserPointer(window));

    // 从用户指针中获取 camera 和 deltaTime
    Camera* camera = appData->camera;
    float deltaTime = *(appData->deltaTime);

    bool Press_W = (key == GLFW_KEY_W && action == GLFW_PRESS);
    bool Press_A = (key == GLFW_KEY_A && action == GLFW_PRESS);
    bool Press_S = (key == GLFW_KEY_S && action == GLFW_PRESS);
    bool Press_D = (key == GLFW_KEY_D && action == GLFW_PRESS);

    camera->processKey(Press_W, Press_A, Press_S, Press_D, deltaTime);
}

int main() {
    Camera camera;
    float deltaTime = 0.0f;

    AppData appData = { &camera, &deltaTime };

    // 初始化 GLFW
    // ...

    // 设置用户指针
    glfwSetWindowUserPointer(window, &appData);
    
    // 设置键盘回调函数
    glfwSetKeyCallback(window, key_callback);

    // 主循环
    // ...
}
```
感觉已经不错了，但是这样还是会有一些额外的操作，想要更进一步包装整个逻辑，后续我们可以用一个InputManager类将类似camera的整个回调处理逻辑包含进去。
将这些回调函数作为InputManager类的静态成员函数，这个类最好使用单例的设计模式。这样在回调函数中想读取或操作任何主函数中的变量，都可以通过InputManager类的接口传入
```cpp  
class InputManager {
public:
    static InputManager& getInstance() {
        static InputManager instance; // 唯一的实例，局部静态变量，第一次调用时创建，线程安全
        return instance;
    }

    // 删除拷贝构造函数和赋值运算符，防止其他人复制或赋值单例实例
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // 设置 camera 和 deltaTime
    void setCamera(Camera* cam) {
        camera = cam;
    }

    void setDeltaTime(float* dt) {
        deltaTime = dt;
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) 
    {
        InputManager& instance = getInstance();  // 获取单例实例

        if (instance.camera) {
            bool Press_W = (key == GLFW_KEY_W && action == GLFW_PRESS);
            bool Press_A = (key == GLFW_KEY_A && action == GLFW_PRESS);
            bool Press_S = (key == GLFW_KEY_S && action == GLFW_PRESS);
            bool Press_D = (key == GLFW_KEY_D && action == GLFW_PRESS);
            
            instance.camera->processKey(Press_W, Press_A, Press_S, Press_D, *(instance.deltaTime));
        }
    }

private:
    InputManager() = default; // 构造函数设为私有，确保不能外部实例化
    Camera* camera = nullptr;
    float* deltaTime = nullptr;
};

int main() {

    InputManager& inputManager = InputManager::getInstance();
    inputManager.setCamera(&camera);
    inputManager.setDeltaTime(&deltaTime);

    // 初始化 GLFW 和创建窗口
    // ...

    // 设置键盘回调
    glfwSetKeyCallback(window, InputManager::keyCallback);

    // 主循环
    // ...
    return 0;
}

```
但是到这里我们运行程序会发现一个问题，按下按键摄像机会动一下，但是按住按键它不会持续移动，因为键盘处理回调函数只会在按下按键时被调用。想要处理按住按键这个操作，我们需要将处理逻辑移动至主函数的循环里，keyCallback就暂时留空，后面有需求再往里添加内容

## Hint