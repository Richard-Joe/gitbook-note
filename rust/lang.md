# 语言基础

## 1. 变量、常量、隐藏

- **`let`** 关键字：声明变量。
	1. 变量默认是不可变的 `Immutable`
	2. 加上**`mut`**，使变量可变

- **`const`** 关键字：声明常量。
	1. 不可以用 `mut`
	2. 必须标注类型
	3. 可以声明在任何作用域
	4. 只可以绑定到常量表达式

- **`Shadowing`** ：隐藏。
	1. 可以使用 `let` 声明 同名新变量，新的变量就会 `shadow` 之前声明的变量
	2. 新的变量，类型可以与之前不同

## 2. 数据类型：标量类型、复合类型

- 标量类型：
	1. 整数：`i8、u8、i16、u16、i32、u32、i64、u64、i128、u128、isize、usize`。默认类型是 `i32`
	> 整数溢出：
	> 	- debug编译：程序在运行时，如果发生溢出，会 panic；
	> 	- release编译：程序在运行时，如果发生溢出，会执行 “环绕” 操作，不会panic；
	3. 浮点：`f32、f64`。默认类型是 `f64`
	4. 布尔：`bool`；值为`true、false`；占用一个字节大小
	5. 字符：`char`；使用单引号；占用**4个字节**大小；是**Unicode标量值**

- 复合类型：
	1. 元组：支持多个类型的值放一起；长度是固定的；`let tup: (i32, f64, char) = (1, 2.0, 'a');`
	2. 数组：每个元素的类型必须相同；长度是固定的；存放在 **`stack`** 上；`let arr: [i32; 3] = [1,2,3];`，使用固定值初始化数组时 `let arr = [3; 5];`

## 3. 函数

- 语句（statement）：执行一些动作的指令
- 表达式（expression）：计算产生一个值
- 函数体：由一系列 `statement` 组成，可选的由一个 `expression` 结束；
- 返回值：`->` 符号后声明函数返回值类型；通常是函数体中最后一个表达式的值；提前返回需使用 `return` 关键字

## 4. 控制流

- `if else`：条件必须是 `bool` 类型
- `loop`
- `while`
- `for`：安全、简洁
- `Range`：`rev` 可以反转 `Range`，`for num in (1..4).rev()`
- `match`

## 5. 所有权

所有权是 `Rust 最核心`的特性，它使得 `Rust` 无需 `GC` 就可以保证内存安全。

所有权存在的原因，就是管理 `heap` 数据：

- 跟踪代码的哪些部分正在使用 `heap` 的哪些数据；
- 最小化 `heap` 上的重复数据量；
- 清理 `heap` 上未使用的数据以避免空间不足；

### 5.1. 所有权规则

- 每个值都有一个变量，这个变量是该值的所有者；
- 每个值同时只能有一个所有者；
- 当所有者超出作用域（scope）时，该值将被删除；

### 5.2. 内存和分配

- **当变量走出作用域时，Rust 会自动调用`drop`函数，将内存还给操作系统**；

- **移动（Move）**：
![move](images/move.png)

把 s1 赋值给 s2 , 为了保证内存安全：

	Rust只在 `stack` 上复制，没有在 `heap` 上复制；
	Rust让 s1 失效；

如果真的想对 `heap` 上的数据做 `深度拷贝`，可以使用 `clone` 方法。

- **复制**：
	- 如果一个类型实现了 `Copy trait`，那么旧的变量在赋值后仍然可用；
	- 如果一个类型或该类型的一部分实现了 `Drop trait`，那么 Rust 不允许让它再去实现 `Copy trait`；

**简单说，stack上的资源有copy trait，heap上的资源没有copy trait。**

一些拥有 Copy trait 的类型：

- 标量类型
- 元组，前提是所有字段都是copy的

### 5.3. 函数

- 值传递给函数时，将发生 **移动** 或 **复制**；
- 函数在返回值的过程中，同样会发生所有权的转移；

一个变量的所有权，总是遵循同样的模式：

- 把一个值赋给其他变量时，就会发生移动；
- 当一个包含 `heap` 数据的变量离开作用域时，它的值就会被 `drop` 函数清理，除非数据的所有权移动到另一个变量上了；

## 6. 引用和借用

- 引用：`&`符号，允许引用某些值而**不取得其所有权**；默认不可变的；
- 借用：把引用作为函数参数这个行为叫做借用；`fn calc_len(s: & String) -> usize`
- 可变引用：比如`mut & String`；**在特定作用域内，某块数据只能有一个可变引用。（避免竞争）**；
	- 不可以同时拥有一个可变引用和一个不可变引用
	- 可以同时拥有多个不可变引用

## 7. 切片（slice）

切片（slice）：`&str`，也是一种**不持有所有权**的数据类型；

- 字符串切片的范围索引必须发生在有效的 UTF-8 字符边界内；
- 如果对一个**多字节的字符**中创建字符串切片，程序报错并退出；

## 8. 结构体（struct）

- 一旦 struct 的实例是可变的，那么实例中的所有字段都是可变的；

```rust
struct User {
    active: bool,
    username: String,
    email: String,
    sign_in_count: u64,
}

fn build_user(email: String, username: String) -> User {
    User { // 作为函数返回值
        active: true,
        username, // 简写
        email,    // 简写
        sign_in_count: 1,
    }
}

let u1 = build_user(String::from("joe@163.com"), String::from("joe"));

let u2 = User {
    email: String::from("tom@163.com"),
    username: String::from("tom"),
    ..u1   // struct 更新语法
};

// Tuple struct
// struct有名，但里面元素没名
struct Point(i32, i32, i32);
let origin = Point(4, 5, 6);

// Unit-Like struct
// 没有任何字段，适用于需要在某个类型上实现某个 trait，但里面有没有需要存储的数据
struct Solution;
```

- **方法**：在 `impl` 块定义方法；方法的第一个参数可以是 `&self`，也可以获得其所有权 或 可变借用；
- **关联函数**：在 `impl` 块定义，不把 `self` 作为第一个参数的函数；

```rust
#[derive(Debug)]
struct Rectangle {
    width: u32,
    height: u32,
}

impl Rectangle {
    // 方法
    fn area(&self) -> u32 {
        self.width * self.height
    }

    // 关联函数
    fn square(size: u32) -> Rectangle {
        Rectangle {
            width: size,
            height: size
        }
    }
}
```

## 9. 枚举（enum）

- 可以将数据附加到枚举的变体中
- 每个变体可以拥有不同的类型以及关联的数据量
- 使用`impl`为枚举定义方法

```rust
#[derive(Debug)]
enum Message {
    Quit,
    Move { x: i32, y: i32 },
    Write(String),
    ChangeColor(i32, i32, i32),
}

impl Message {
    fn call(&self) {
        println!("{:?}", self);
    }
}
```

Rust 提供了类似 `Null` 概念的枚举：`Option<T>`

- `Option<T>` 和 `T` 是不同的类型；
- 若需要使用`Option<T>`中的`T`，需要先转换；

```rust
enum Option<T> {
    Some(T),
    None,
}
```

## 10. match

- 允许一个值与一系列模式进行匹配，并执行匹配的模式对应的代码
- `模式`可以是字面值、变量名、通配符...
- 模式可以绑定值，比如从 enum 变体中提取值；
- match 匹配**必须穷举所有的可能**；如果不想处理，则使用 `_ 通配符`来替代其余值；

```rust
let x = Some(5);
match x {
    None => None,
    Some(i) => Some(i + 1),
}

let v = 0u8;
match v {
    1 => 1,
    3 => 3,
    5 => 5,
    _ => 999,
}
```

## 11. if let

- 只**关心一种匹配**而忽略其他匹配的情况；
- 放弃了穷举的可能；

```rust
let x = Some(5);
if let Some(5) = x {
    println!("five")
} else {
	println!("others")
}
```

## 12. Package、Crate、Module、Path

- `Crate`：
	- 两种类型：`binary`、`library`
	- `Crate Root`：指源代码入口文件
		- `src/main.rs`  ( binary )
		- `src/lib.rs`  ( library )

- `Package`：
	- `Cargo.toml`：描述了如果构建 `Crates`
	- 只能包含 `0-1` 个 `library crate`
	- 可以包含 `任意数量` 的 `binary crate`
	- 但必须至少包含一个 `crate`

- `Module`：在一个 `crate` 内，将代码进行分组
	- 控制作用域和私有性（public、private）
	- **`mod`** 关键字，可嵌套

- `Path`：
	- 两种形式：绝对路径（推荐）、相对路径
	- 标识符 `::`
	- **`super`** 关键字，用来访问父级模块路径中的内容，类似文件系统中的`..`

- `privacy boundary`：私有边界
	- Rust中所有条目（函数、方法、struct、enum、模块、常量）**默认是私有的**。
	- 父模块无法访问子模块的私有条目
	- 子模块可以使用所有祖先模块中的条目
	- **`pub`** 关键字标记条目为公共的

- **`pub struct`**：
	- `struct` 是公共的，但**里面的字段默认是私有的**
	- 里面的字段需要单独设置 `pub` 来变成公有的

- **`pub enum`**：
	- `enum` 是公共的，
	- **`enum` 的变体也都是公共的**

- **`use`**：将 `path` 导入到作用域内，（作用域内**私有**）
	- 函数：惯用做法是将函数的父级模块引入作用域（指定父级）
	- `struct、enum`：惯用做法指定完整路径（指定本省）（除非两个模块中的结构体名称相同）
	- **`as`** 关键字：可以给引入路径指定（本地）别名
	- **`pub use`**：重新导出名称
	- 可以使用 **嵌套路径** 清理大量的 `use` 语句
	- 支持通配符 `*`

```rust
// 嵌套路径
use std::{cmp::Ordering, io}
use std::io::{self, Write}
```

- **将模块内容移动到其他文件**
	- 模块名后面是 `;`
	- Rust 会从与模块同名的文件中加载内容

## 13. Vector、String、HashMap

- Vector

```rust
// Vector
let mut v: Vec<i32> = Vec::new();
let mut v1 = vec![1,2,3];
v.push(99);
v.get(0);
for item in  v {}
for (i, item) in v.iter().enumerate() {}

enum Cell {
    Int(i32),
    Float(f64),
    Text(String),
}
let row = vec![
    Cell::Int(1),
    Cell::Float(2.34),
    Cell::Text(String::from("red")),
]
```

- String：**字符串在Rust中使用 `UTF-8` 编码**
	- `String`  来自标准库
	- `&str` 字符串切片，来自Rust核心语言
	- 其他字符串类型：`OsString、OsStr、CString、CStr`
	- **Rust中字符串不支持索引语法访问**。原因：
		- 1. 不安全，越界问题；
		- 2. 索引操作应消耗O(1)，但String无法保证这个时间，因为需要遍历所有内容才能确定有多少合法字符；
	- `String` 是对 `Vec<u8>` 的包装
	- Rust中看待字符串有三种方式：
		- 字节（Bytes）
		- 标量值（Scalar Values）
		- 字形簇（Grapheme Clusters）
	- Rust中，**字符串中的字符所占的字节数是变化的**（**`1-4`个字节**）
	- 切割字符串，必须谨慎使用，**如果跨越了字符边界，程序就会panic**。

```rust
let blue = "Blue".to_string();
let mut red = String::from("Red");
red.push_str("black");
red.push_str(&blue);  // 附加字符串切片
red.push('😂');      // 附加单个字符

let color = red + &blue; // fn add(self, s: &str) -> String
// 注意：red发生了移动，所有权进行了转移，后续无法继续使用

let s = format!("{}-{}", "foo", "bar");

// 字符串是 UTF-8 编码
println!("{}", "a".len());  // 打印 1
println!("{}", "д".len());  // 打印 2
println!("{}", "व".len());  // 打印 3
println!("{}", "😂".len()); // 打印 4

// 遍历
for i in color.chars() {}  // 想得到 标量值
for i in color.bytes() {}  // 想得到 字节
```

- HashMap
	- 在元素类型为 `Tuple`（两个值） 的 `Vector` 上使用 `collect`方法，可以组建一个 `HashMap` 
	- **所有权**：
		- **对于实现了 `Copy trait` 的类型，值会被复制到 `HashMap` 。**比如 `i32` 
		- **对于拥有所有权的值，值会被移动，所有权会转移给 `HashMap` 。**比如 `String` 
		- 如果将**值的引用**插入到 `HashMap` ，值本身不会移动；但这期间必须保持引用的值有效
	- **Hash函数**：
		- 默认情况下的hash函数，可以抵抗Dos攻击，安全性更好，但不是最快的算法；
		- 可以指定不同的 `hasher` （指实现了 `BuildHasher trait` 的类型）来切换；

```rust
let mut map: HashMap<i32, String> = HashMap::new();
map.insert(10, String::from("red"));
let v = map.get(&10);
match v {
    None => println!("not exist"),
    Some(s) => println!("{}", s),
}

let color = vec![String::from("red"), String::from("blue")];
let nums = vec![100, 200];
let mut map: HashMap<_, _> = color.iter().zip(nums.iter()).collect();
for (k,v) in &map {
    println!("{}: {}", k, v)
}
map.entry(&String::from("black")).or_insert(&300); // key不存在，则插入

let text = "hello world wonderful world";
let mut map = HashMap::new();
for word in text.split_whitespace() {
    let count = map.entry(word).or_insert(0);
    *count += 1;
}
println!("{:#?}", map);
// entry方法：返回 enum Entry，代表值是否存在
// or_insert方法：
//        如果 key 存在，返回 value 的可变引用；
//        如果 key 不存在，将新键值(k,v)插进去，返回 value 的可变引用；
```

## 14. 错误处理

- 可恢复：`Result<T, E>`
- 不可恢复：`panic!` 宏

**panic：**

- 默认情况下，当 panic 发生时，程序会 **展开（unwind）** 调用栈；（Rust沿调用栈往回走，清理每一个遇到的数据）
- 想让二进制文件更小，可以把这一行为改为 **立即中止（abort）** 调用栈；（由OS进行清理）
	- 在Cargo.toml中设置：
```toml
[profile.release]
panic = 'abort'
```
- 回溯信息：设置环境变量 `RUST_BACKTRACE`；debug模式；

**Result枚举：**

```rust
enum Result<T, E> {
    Ok(T),
    Err(E),
}
```

**错误处理：**

- **`match`**
- **`unwrap`**：match表达式的一个快捷方式；
- **`expect`**：可指定错误信息

```rust
let f = File::open("hello.ext").unwrap();
let f = File::open("hello.ext").expect("cannot open file");
```

**传播错误：**

- **`?` 运算法**：传播错误的快捷方式
- **`from` 函数**：用于错误之间的转换；场景：针对不同的错误原因，返回同一种错误类型；
	- 要求：每个错误类型都实现了转换为所返回的错误类型的from函数；
- **`main` 函数**的返回类型是 `()`，`main` 函数的返回类型也可以是 `Result<T, E>`
	- main 返回类型写：`Result<(), Box<dyn Error>>`
	- `Box<dyn Error>` 是 trait 对象，表示任何可能的错误类型；

```rust
fn open_file() -> Result<String, io::Error> {
    let f = File::open("hello.ext")?;
    Ok(String::from("ok"))
}

fn read_file() -> Result<String, io::Error> {
    let mut s = String::new();
    File::open("hello.ext")?.read_to_string(&mut s)?;
    Ok(s)
}
```

## 15. 泛型

- 使用泛型的代码和使用具体类型的代码运行速度是一样的。
- 单态化（monomorphization）：在编译时将泛型替换为具体类型

```rust
// 函数
fn largest<T>(list: &[T]) -> T {}

// 结构体
struct Point<T, U> {
    x: T,
    y: U,
}

// 枚举
enum Result<T, E> {
    Ok(T),
    Err(E),
}

// 方法
impl<T, U> Point<T, U> {
    fn mixup<V, W>(self, other: Point<V, W>) -> Point<T, W> {}
}
```

## 16. Trait

- 类似`Interface`，抽象的定义共享行为
- `Trait bounds`：泛型类型参数指定为实现了特定行为的类型
- `trait` 可以有多个方法，每个方法签名占一行；
- 实现该 `trait` 的类型必须提供具体的方法实现；
- 实现 `trait` 的约束：**这个类型 或 这个 trait 是在本地 `crate` 里定义的**，所以**无法为外部类型来实现外部的 trait**；
- 可以在 `trait` 里面定义**默认实现的方法**；**默认实现的方法可以调用 trait 中其他的方法，即使这个方法没有默认实现**；
- **无法从方法的重写实现里面调用默认的实现**；
- `trait` 可以**作为参数**，三种写法：1. impl、2. trait bound、3. trait bound where
- `trait` 可以**作为返回类型**；
	- **`impl Trait` 只能返回一种确定的类型**，返回可能不同的类型会报错；
- 可以使用`Trait bound` **有条件的实现方法**
- **覆盖实现（blanket implementations）**：可以为满足`Trait bounds`的所有类型上实现`trait`

```rust
// 定义 trait
pub trait Summary {
    fn summarize(&self) -> string;

    fn summarize_default(&self) -> string {
        format!("default {}", self.summarize())
    }
}

pub struct NewsArticle {...}

// 实现 trait
impl Summary for NewsArticle {
    fn summarize(&self) -> string {
        format!("{}", "article")
    }
}

// trait 作为参数
pub fn notify(item: impl Summary) {
    println!("news {}", item.summarize())
}
// 要求 item 实现了多个 trait
pub fn notify1(item: impl Summary + Display) {
    println!("news {}", item.summarize())
}

// Trait bound 写法
pub fn notify<T: Summary + Display>(item1: T, item2: T) {
    println!("news {}", item.summarize())
}

// Trait bound 使用 where
pub fn notify<T, U>(a: T, b: U) -> string
where
    T: Summary + Display,
    U: Clone + Debug,
{
    println!("news {}", item.summarize())
}

// trait 作为返回类型
pub fn notify(s: &str) -> impl Summary {
    NewsArticle {...}
}

// Trait bound 有条件的实现方法
struct Pair<T> {
    x: T,
    y: T,
}
impl<T> Pair<T> {
    // 所有的Pair类型，都有new函数
    fn new(x: T, y: T) -> self{
        Self { x, y }
    }
}
impl<T: Display + PartialOrd> Pair<T> {
    // 只有实现了(Display+PartialOrd)这两个trait的Pair类型，才拥有cmp函数
    fn cmp(&self) {...}
}

// 标准库 string.rs 中，使用了覆盖实现
// 为所有实现了Display这个trait的类型T，实现ToString这个trait
impl<T: Display> ToString for T {
    default fn to_string(&self) -> string {}
}
```

## 17. 生命周期

- Rust 的每个引用都有自己的生命周期
- 生命周期：引用保持有效的作用域
- 大多数情况下，生命周期是隐式的、可被推断的；当无法被编译器推断时，需要手动标注生命周期
- 生命周期存在的主要目标：避免悬垂引用（dangling reference）
- 借用检查器：比较作用域来判断所有的借用是否合法
- 生命周期的标注不会改变引用的生命周期长度
- 当指定了泛型生命周期参数，函数可以接收带有任何生命周期的引用
- 生命周期的标注：描述了多个引用的生命周期的关系，但不影响生命周期
- 静态生命周期：整个程序的持续时间。`let s: &'static str = "hello";`

```rust
fn longest<'a>(x: &'a str, y: &'a str) -> &'a str {
	if x > y {
		x
	} else {
		y
	}
}

struct ImportantExcerpt<'a> {
	part: &'a str,
}

impl<'a> ImportantExcerpt<'a> {
	fn level(&self) -> i32 {
		3
	}
}

fn longest_with_an_annoucement<'a, T>
    (x: &'a str, y: &'a str, ann: T) -> &'a str
where
    T: Display,
{
	println!("Announcement! {}", ann)
	if x > y {
		x
	} else {
		y
	}
}
```

生命周期省略规则：

- 编译器考虑的一些特殊情况，无需显示标注
- 如果编译器无法推断，则编译错误
- 输入生命周期：函数/方法的参数
- 输出生命周期：函数/方法的返回值
- 规则适用于 fn 定义和 impl 块

三个省略规则：

1. 每个引用类型的参数都有自己的生命周期；
2. 如果只有 1 个输入生命周期参数，那么该生命周期被赋给所有的输出生命周期参数；
3. 如果有多个输入生命周期参数，但其中一个是 &self 或 &mut self（方法），那么 self 的生命周期会被赋给所有的输出生命周期参数；


## 18. 测试

- 测试：`#[cfg(test)]`、`#[test]`
- 断言：`assert!`、`assert_eq!`、`assert_ne!`
- 恐慌：`#[should_panic]`
- 可使用 `Result<T, E>` 作为返回类型
- 默认 `cargo test` 并行运行测试，控制线程数量 `--test-threads`
- 测试通过，不会看到打印到标准输出的内容；否则可以看到
- 指定测试名称运行测试：`cargo test test-name`
- 忽略测试：`#[ignore]`
- 只运行被忽略的测试：`cargo test -- --ignored`
- 集成测试：`tests`目录
- 环境变量：`env::var("xx")`
- 标准输出：`println!`
- 标准错误：`eprintln!`

## 19. 闭包

- 匿名函数，可以保存为变量、作为参数
- 闭包不要求标注参数和返回值的类型，编译器通常能推断出类型；但也可以手动添加类型标注
- 记忆化（memoization）：创建一个 struct，**它持有闭包及其调用结果**
	- struct 中需要指明闭包的类型
	- **每个闭包实例都有自己唯一的匿名类型**，即使两个闭包签名完全一样
	- 需要使用：**泛型和 Trait Bound**
- **所有的闭包都至少实现了以下 trait 之一**：
	- Fn
	- FnMut
	- FnOnce
- **闭包可以访问定义它的作用域内的变量，而普通函数则不能**
	- 会产生内存开销
	- 闭包从所在环境中捕获值的方式：
		- 取得所有权：FnOnce
		- 可变借用：FnMut
		- 不可变借用：Fn
	- Rust如何推断具体使用哪个 trait：
		- 所有的闭包都实现了 FnOnce
		- 没有移动捕获变量的实现了 FnMut
		- 无需可变访问捕获变量的闭包实现了 Fn
- `move` 关键字：**可以强制闭包取得它所使用的环境值的所有权**
	场景：当将闭包传递给新线程以移动数据使其归新线程所有时
- 最佳实践：当指定 Fn trait bound 之一时，首先用 Fn，基于闭包体里的情况，编译器会再告诉你需要使用 FnOnce 或 FnMut

```rust
let c1 = |x: u32| -> u32 { x + 1 };
let c2 = |x| { x + 1 };
let c3 = |x| x + 1;

let x = vec![1,2,3];
let eq_x = move |z| z == x;
// x 已发生移动，这后面无法再使用x
```

```rust
struct Cacher<T>
where
    T: Fn(u32) -> u32,
{
    calculation: T,
    map: HashMap<u32, u32>,
}

impl<T> Cacher<T>
where
    T: Fn(u32) -> u32,
{
    fn new(calculation: T) -> Cacher<T> {
        Cacher {
            calculation,
            map: HashMap::new(),
        }
    }

    fn value(&mut self, key: u32) -> u32 {
        let v = self.map.get(&key);
        match v {
            Some(i) => *i,
            None => {
                let calc = (self.calculation)(key);
                self.map.insert(key, calc);
                calc
            }
        }
    }
}

fn main() {
    let mut closure = Cacher::new(|num| {
        println!("calculation slowly...");
        thread::sleep(Duration::from_secs(2));
        num
    });

    println!("one {}", closure.value(99));
    println!("two {}", closure.value(99));
    println!("three {}", closure.value(99));
    println!("one {}", closure.value(100));
    println!("two {}", closure.value(100));
}
```

## 20. 迭代器

- 所有的迭代器都实现了 `Iterator trait`
	- 实现 `Iterator trait` 需要定义个 Item 类型，它用于 next 方法的返回类型
	- next 每次返回迭代器中的一项，返回结果包裹在 Some 里，迭代结束则返回 None
- `iter` 方法：在不可变引用上创建迭代器
- `into_iter` 方法：创建的迭代器会获得所有权
- `iter_mut` 方法：迭代可变的引用
- 消耗迭代器的方法：next、sum等
- 迭代器适配器：把迭代器转换为不同种类的迭代器；比如 map、filter

```rust
pub trait Iterator {
	type Item;
	fn next(&mut self) -> Option<Self::Item>;
}

let v1 = vec![1, 3, 4, 6, 7, 9, 10];
let it = v1.iter().filter(|x| *x % 2 == 0);
```

```rust
struct Counter {
    count: u32,
}

impl Counter {
    fn new() -> Counter {
        Counter { count: 0 }
    }
}

impl Iterator for Counter {
    type Item = u32;

    fn next(&mut self) -> Option<Self::Item> {
        if self.count < 5 {
            self.count += 1;
            Some(self.count)
        } else {
            None
        }
    }
}

fn main() {
    let s: u32 = Counter::new()
        .zip(Counter::new().skip(1))
        .map(|(a, b)| a * b)
        .filter(|x| x % 3 == 0)
        .sum();

    println!("{}", s);
}
```

## 21. 发布

Cargo 主要有两个 profile：

- dev profile：适用于开发，cargo build
- release profile：适用于发布，cargo build --release

自定义 profile时，在 `Cargo.toml` 里添加 `[profile.xxx]` 区域，覆盖默认配置的子集。

crate 官方注册表：`https://crates.io` 

- 登录账号：cargo login [token]
- 发布：cargo publish （一旦发布，该版本代码无法覆盖，无法删除）
- 撤回一个版本：cargo yank --vers 1.0.1 
- 取消撤回：cargo yank --vers 1.0.1 --undo
- 安装二进制 crate：cargo install

文档注释：生成HTML文档；使用 `///` ；支持 Markdown

- `cargo doc`、`cargo dock --open`：生成在 `target/doc` 目录下
- 常用章节：
	- `# Examples`：cargo test 可以把文档注释中的示例代码作为测试来运行
	- `# Panics`：函数可能发生 panic 
	- `# Errors`：如果函数返回 Result，描述可能的错误种类，以及可导致错误的条件
	- `# Safety`：如果函数处于 unsafe 调用，就应该解释函数 unsafe 的原因，以及调用者确保的使用前提

如果是描述 crate 和模块的注释，或记录一个模块整体，使用 `//!`

使用 `pub use` 导出方便使用的公共 API

工作空间：

- 管理多个关联且需协同开发的crate
- 就是一套共享同一个 Cargo.lock 和输出文件夹的包

自定义命令扩展 cargo：

- 二进制 cargo-abc；运行命令`cargo abc`
- cargo --list

