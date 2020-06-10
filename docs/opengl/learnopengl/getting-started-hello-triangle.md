> [!COMMENT]
> 在学习此节之前，建议将这三个单词先记下来：
> - 顶点数组对象：Vertex Array Object，VAO
> - 顶点缓冲对象：Vertex Buffer Object，VBO
> - 索引缓冲对象：Element Buffer Object，EBO 或 Index Buffer Object，IBO

OpenGL 中，任何事物都在 3D 空间中，而屏幕和窗口却是 2D 像素数组，这导致 OpenGL 的大部分工作都是关于把 3D 坐标转变为适应你屏幕的 2D 像素。3D 坐标转为 2D 坐标的处理过程是由 OpenGL 的**图形渲染管线**（Graphics Pipeline，也译为管线）管理的。图形渲染管线可以被划分为两个主要部分：第一部分把你的 3D 坐标转换为 2D 坐标，第二部分是把 2D 坐标转变为实际的有颜色的像素。

图形渲染管线可以被划分为几个阶段，每个阶段将会把前一个阶段的输出作为输入。所有这些阶段都是高度专门化的（它们都有一个特定的函数），并且很容易*并行*执行。正是由于并行执行的特性，当今大多数显卡都有成千上万的小处理核心，它们在 GPU 上为每一个阶段运行各自的小程序，从而在图形渲染管线中快速处理你的数据。这些小程序叫做**着色器**（Shader）。

下图为图形渲染管线的每个阶段的抽象展示。蓝色部分代表的是我们可以注入自定义的着色器的部分（*有些着色器允许开发者自己配置，这就允许我们用自己写的着色器来替换默认的。这样我们就可以更细致地控制图形渲染管线中的特定部分了*）。

![](_images/learnopengl-getting-started-8.png）

按照上图，我们概括性地解释一下渲染管线的每个部分：

1. 首先，我们以数组的形式传递 3 个 3D 坐标作为图形渲染管线的输入，用来表示一个三角形，这个数组叫做顶点数据（Vertex Data）；顶点数据是一系列顶点的集合。一个**顶点**（Vertex）是一个 3D 坐标的数据的集合。而顶点数据是用**顶点属性**（Vertex Attribute）表示的，它可以包含任何我们想用的数据，但是简单起见，我们假定每个顶点的顶点属性只由一个 3D 位置和一些颜色值组成。

> [!NOTE]
> 为了让 OpenGL 知道我们的坐标和颜色值构成的到底是什么，OpenGL 需要你去指定这些数据所表示的渲染类型。我们是希望把这些数据渲染成一系列的点？一系列的三角形？还是仅仅是一条长长的线？做出的这些提示叫做**图元**（Primitive），任何一个绘制指令的调用都将把图元传递给 OpenGL。这是其中的几个：`GL_POINTS`、`GL_TRIANGLES`、`GL_LINE_STRIP`。

2. 图形渲染管线的第一个部分是**顶点着色器**（Vertex Shader），它把一个单独的顶点作为输入。顶点着色器主要的目的是把 3D 坐标转为另一种 3D 坐标（后面会解释），同时顶点着色器允许我们对顶点属性进行一些基本处理。
3. **图元装配**（Primitive Assembly）阶段将顶点着色器输出的所有顶点作为输入（如果是 `GL_POINTS`，那么就是一个顶点），并所有的点*装配*成指定图元的形状；本节例子中是一个三角形。
4. 图元装配阶段的输出会传递给**几何着色器**（Geometry Shader）。几何着色器把图元形式的一系列顶点的集合作为输入，它可以通过产生新顶点构造出新的（或是其它的）图元来生成其他形状。例子中，它生成了另一个三角形。
5. 几何着色器的输出会被传入**光栅化阶段**（Rasterization Stage），这里它会把图元映射为最终屏幕上相应的像素，生成供片段着色器（Fragment Shader）使用的片段（Fragment，*OpenGL 中的一个片段是 OpenGL 渲染一个像素所需的所有数据*）。在片段着色器运行之前会执行**裁切**（Clipping）。裁切会丢弃超出你的视图以外的所有像素，用来提升执行效率。
6. **片段着色器**的主要目的是计算一个像素的最终颜色。通常，片段着色器包含 3D 场景的数据（比如光照、阴影、光的颜色等等），这些数据可以被用来计算最终像素的颜色。
7. 在所有对应颜色值确定以后，最终的对象将会被传到最后一个阶段，我们叫做 **Alpha 测试**和**混合**（Blending）阶段。这个阶段检测片段的对应的深度（和模板）值（后面会讲），用它们来判断这个像素是其它物体的前面还是后面，决定是否应该丢弃。这个阶段也会检查 alpha 值（alpha 值定义了一个物体的透明度）并对物体进行混合（Blend）。所以，即使在片段着色器中计算出来了一个像素输出的颜色，在渲染多个三角形的时候最后的像素颜色也可能完全不同。

对于大多数场合，我们只需要配置顶点和片段着色器就行了。几何着色器是可选的，通常使用它默认的着色器就行了。

### 顶点输入

开始绘制图形之前，我们必须先给 OpenGL 输入一些顶点数据。OpenGL 是一个 3D 图形库，所以我们在 OpenGL 中指定的所有坐标都是 3D 坐标（x、y 和 z）。OpenGL 仅当 3D 坐标在 3 个轴（x、y 和 z）上都为 - 1.0 到 1.0 的范围内时才处理它。所有在所谓的标准化设备坐标（Normalized Device Coordinates）范围内的坐标才会最终呈现在屏幕上。

由于我们希望渲染一个三角形，我们一共要指定三个顶点，每个顶点都有一个 3D 位置。我们会将它们以标准化设备坐标的形式（OpenGL 的可见区域）定义为一个 `float` 数组。

```c
float vertices[] = {
    // z 坐标设置为 0.0，这样三角形每一点的深度都是一样的，从而使它看上去像是 2D 的
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};
```

> [!NOTE|label:标准化设备坐标（Normalized Device Coordinates）]
> 一旦顶点坐标在顶点着色器中处理过，它们就应该是**标准化设备坐标**了，标准化设备坐标是一个 x、y 和 z 值在 - 1.0 到 1.0 的一小段空间。任何落在范围外的坐标都会被丢弃/裁剪。下面会看到我们定义的在标准化设备坐标中的三角形（忽略 z 轴）：
>
> ![](_images/learnopengl-getting-started-9.png)
>
> 标准化设备坐标接着会变换为**屏幕空间坐标**（Screen-space Coordinates），这是使用你通过 `glViewport` 函数提供的数据，进行**视口变换**（Viewport Transform）完成的。所得的屏幕空间坐标又会被变换为片段输入到片段着色器中。

定义好顶点数据以后，我们会把它作为输入发送给顶点着色器。它会在 GPU 上创建*内存*用于储存我们的顶点数据，还要配置 OpenGL 如何解释这些内存，并且指定其如何发送给显卡。顶点着色器接着会处理我们在内存中指定数量的顶点。

### 顶点着色器

### 编译着色器

### 片段着色器