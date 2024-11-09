1. 单例模式 (Singleton)
单例模式确保一个类只有一个实例，并提供一个全局访问点。适用于需要唯一对象（如日志管理器、配置管理器）的情况。

```cpp
class Singleton {
private:
    static Singleton* instance;
    Singleton() {} // 构造函数私有化

public: 
    static Singleton* getInstance() {
        if (!instance) {
            instance = new Singleton();
        }
        return instance;
    }
};

// 初始化静态成员
Singleton* Singleton::instance = nullptr;
```
2. 工厂模式 (Factory)
工厂模式用于创建对象，而无需指定创建的具体类。适用于根据条件创建不同类的情况。

```cpp
class Product {
public:
    virtual void use() = 0;
};

class ConcreteProductA : public Product {
public:
    void use() override {
        std::cout << "Using Product A" << std::endl;
    }
};

class ConcreteProductB : public Product {
public:
    void use() override {
        std::cout << "Using Product B" << std::endl;
    }
};

class Factory {
public:
    static Product* createProduct(const std::string& type) {
        if (type == "A") return new ConcreteProductA();
        if (type == "B") return new ConcreteProductB();
        return nullptr;
    }
};
```
3. 观察者模式 (Observer)
观察者模式定义了对象间的一对多依赖关系，一个对象的状态变化时，所有依赖的对象都会自动收到通知。适用于事件驱动的系统。

```cpp
#include <vector>
#include <iostream>

class Observer {
public:
    virtual void update(int state) = 0;
};

class ConcreteObserver : public Observer {
public:
    void update(int state) override {
        std::cout << "Observer received state: " << state << std::endl;
    }
};

class Subject {
private:
    std::vector<Observer*> observers;
    int state;

public:
    void attach(Observer* observer) {
        observers.push_back(observer);
    }

    void setState(int newState) {
        state = newState;
        notify();
    }

    void notify() {
        for (Observer* observer : observers) {
            observer->update(state);
        }
    }
};
```
4. 装饰器模式 (Decorator)
装饰器模式用于在不修改类定义的情况下给类添加功能。适用于动态扩展对象功能的情况。

```cpp
class Component {
public:
    virtual void operation() = 0;
};

class ConcreteComponent : public Component {
public:
    void operation() override {
        std::cout << "Concrete Component Operation" << std::endl;
    }
};

class Decorator : public Component {
protected:
    Component* component;

public:
    Decorator(Component* comp) : component(comp) {}

    void operation() override {
        component->operation();
    }
};

class ConcreteDecorator : public Decorator {
public:
    ConcreteDecorator(Component* comp) : Decorator(comp) {}

    void operation() override {
        Decorator::operation();
        std::cout << "Concrete Decorator Operation" << std::endl;
    }
};
```
5. 策略模式 (Strategy)
策略模式定义了算法族，允许它们之间互相替换。使得算法可以独立于使用它们的客户端变化。

```cpp
class Strategy {
public:
    virtual void execute() = 0;
};

class ConcreteStrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A" << std::endl;
    }
};

class ConcreteStrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B" << std::endl;
    }
};

class Context {
private:
    Strategy* strategy;

public:
    void setStrategy(Strategy* newStrategy) {
        strategy = newStrategy;
    }

    void executeStrategy() {
        if (strategy) {
            strategy->execute();
        }
    }
};
```