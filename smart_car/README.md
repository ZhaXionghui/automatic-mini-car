# 无人驾驶

## 1.数据采集以及数据预处理

1.arduino烧录小车网页控制和摄像头代码。

<img src="picture\image-20220826194412500.png" alt="image-20220826194412500" style="zoom:67%;" />

2.网页遥控小车按地图行驶，并定时拍照。

<img src="picture\image-20220826194304322.png" alt="image-20220826194304322" style="zoom: 67%;" />

3.收集拍摄的照片，初步筛选数据，将重复度高，成像不清等不符合训练要求的图像删除。

4.登录Easydata，新建图像分割-语义分割数据集，将未打标签的照片上传。

<img src="picture\image-20220822134929390.png" alt="image-20220822134929390" style="zoom:67%;" />

5.创建完成后，进入Easydata的“查看与标签”功能，标记出图像中的background和grass，这样剩下的部分即为road。**需要注意的是建立标签的顺序，这里演示的为先新建background标签，再建grass标签，这一点在后面生成masks图像的时候会稍有影响。**

<img src="picture\image-20220822135135007.png" alt="image-20220822135135007" style="zoom:67%;" />

<img src="picture\image-20220822135609714.png" alt="image-20220822135609714" style="zoom:67%;" />

6.标注完成后，点击开始导出后，待导出完成即可下载到本地。

<img src="picture\image-20220822140330817.png" alt="image-20220822140330817" style="zoom:67%;" />

<img src="picture\image-20220822140422371.png" alt="image-20220822140422371" style="zoom:67%;" />

导出的压缩包解压后如图所示，有两个文件夹和一个txt文件，其中“JPEGAImages”文件夹内部是原图数据，"SegmentationClass"文件夹里面是标签数据图片，txt文件里面是

![image-20220822140542615](picture\image-20220822140542615.png)

## 2.软件安装与环境配置

  1.安装VScode和anaconda3，打开电脑command，输入“conda”+回车，检查anaconda是否已经添加到环境中，若报错没有该命令，则需要将anaconda3对应目录下的的 ..\ANACONDA\condabin 路径和 ..\ANACONDA\Library\bin 路径添加到系统环境中，可以在系统设置中搜索“环境”。

  2.在command中输入：

```
conda create -n name python=3.7  
```

新建虚拟环境，其中name是自行设置的环境名，可以指定python版本，这里以python3.7为例                 <img src="picture\image-20220822101828295.png" style="zoom:67%;" />

 3.command输入：

```
conda activate name    #进入虚拟环境
```

<img src="picture\image-20220822102607980.png" alt="image-20220822102607980" style="zoom:67%;" />

 4.在虚拟环境中安装依赖的python库：在项目文件中有一个packages.txt文件，里面包含了本项目所需要的全部python库的名称，你可以下载下来本文件后，command cd 至指定目录，运行：

```
pip install -r packages.txt -i https://pypi.tuna.tsinghua.edu.cn/simple  #清华源
```

<img src="picture\image-20220822102718800.png" alt="image-20220822102718800" style="zoom: 67%;" />

即可一键配置好本项目所用python环境。

<img src="picture\image-20220822105323007.png" alt="image-20220822105323007" style="zoom:67%;" />

<img src="picture\image-20220822105420411.png" alt="image-20220822105420411" style="zoom:67%;" />

5.VScode安装python插件：点击左侧插件栏，搜索python，一般第一个即为微软认证的python插件，点击下载安装即可。

<img src="picture\image-20220822143529033.png" alt="image-20220822143529033" style="zoom:67%;" />

6.获取配置好的虚拟环境的python路径：

```
where python
```

<img src="picture\image-20220822142132912.png" alt="image-20220822142132912"  />

将获取的第一个python路径复制下来，打开VScode的设置

<img src="picture\image-20220822141859765.png" alt="image-20220822141859765" style="zoom:67%;" />



<img src="picture\image-20220822141937088.png" alt="image-20220822141937088" style="zoom:67%;" />

在设置搜索栏搜索“python interpreter”，将刚刚复制的路径粘贴在“工作区”的Default Interpreter Path下，这项指仅给当前项目配置该环境，当然也可以把用户的Default Interpreter Path也粘贴上该路径，设置您默认使用的环境，依据您自己的需要而定。

<img src="picture\image-20220822142551695.png" alt="image-20220822142551695" style="zoom:80%;" />

## 3.Mask

```
#PATH：\car_project\mask_get
```

1.查看easydata导出的数据：

![image-20220822162451878](picture\image-20220822162451878.png)

项目对应的目录：（这里以github中的项目路径为例，请以您的实际情况为准）：

```
\car_project\mask_get                                #mask_get项目总路径
\car_project\mask_get\photos                         #图片数据总路径
\car_project\mask_get\photos\photos_original         #原图片数据路径
\car_project\mask_get\photos\photos_labeled          #标签图片路径
\car_project\mask_get\photos\masks                   #mask图片生成路径
```

对应代码中的相对路径，直接将相应的图片移入对应目录即可：

```python
#PATH：\car_project\mask_get\mask_rgb_to_gray.py
................................
dataset_dir = './photos/ '                #图片数据的相对路径
image_dir = 'photos_original/'            #原图片的相对路径
rgb_mask_dir = 'photos_labeled/'          #标签图片所在目录
gray_mask_dir = 'masks/'                  #mask图片文件生成目录
................................
```

2.用VScode打开项目目录，目录配置好了之后，直接运行就可以在mask目录下生成mask图片数据。

![image-20220822173350415](picture\image-20220822173350415.png)

需要注意的是，如果您第一个新建的标签为grass，图中红框标注代码应改为：

```python
grass_layer = rgb_mask[:, :, 1] / 128         
background_layer = rgb_mask[:, :, 2] / 128 
```

3.生成mask图片数据后，您可以用temp.py简单检验一下,随机选一个mask，将其路径填入函数中：

```python
#\car_project\mask_get\temp.py
import cv2
import matplotlib.pyplot as plt

mask = cv2.imread('path of a mask',cv2.COLOR_BGR2GRAY)
plt.imshow(mask, 'gray')
plt.show()
```

运行，再打开对应的照片比对一下：

![image-20220822175048296](picture\image-20220822175048296.png)

红框所圈起来的数字：0对应road；1对应grass；2对应background，确保验证无误。

## 4.Unet神经网络

```
#PATH：\car_project\unet_torch
```

  Unet 发表于 2015 年，属于 FCN 的一种变体。Unet 的初衷是为了解决生物医学图像方面的问题，由于效果确实很好后来也被广泛的应用在语义分割的各个方向，比如卫星图像分割，工业瑕疵检测等。

  Unet 跟 FCN 都是 Encoder-Decoder 结构，具体表现为，先对图像进行下采样（池化、卷积等），提取特征，再进行上采样（转置卷积、上池化等），将特征分配给每一个像素点。Encoder 负责特征提取，Decoder负责将特征图恢复原始分辨率。

  1.我们可以通过训练的主程序了解一下，训练无人驾驶模型的代码的结构（如注释所示）

```python
#PATH：\car_project\unet_torch\train.py
def train_net(net,
              device,                        #参数对应为：
              epochs: int = 5,               #迭代次数
              batch_size: int = 1,           #批量大小
              learning_rate: float = 1e-5,   #学习率 
              val_percent: float = 0.1,      #验证集比例
              save_checkpoint: bool = True,  #储存checkpoint二进制文件
              img_scale: float = 0.5,        #缩放大小
              amp: bool = False):
    # 1.创建数据集 装载图片与mask标签
    try:
        dataset = CarvanaDataset(dir_img, dir_mask, img_scale)
    except (AssertionError, RuntimeError):
        dataset = BasicDataset(dir_img, dir_mask, img_scale)

    # 2.将数据集分为验证集与训练集 
    n_val = int(len(dataset) * val_percent)  
    n_train = len(dataset) - n_val
    train_set, val_set = random_split(dataset, [n_train, n_val], generator=torch.Generator().manual_seed(0))     #随机选出一定比例的验证集和训练集

    # 3.创建数据装载器 
    loader_args = dict(batch_size=batch_size, num_workers=1, pin_memory=False)  #num_workers:多线程读取数据 
    train_loader = DataLoader(train_set, shuffle=True,**loader_args)
    val_loader = DataLoader(val_set, shuffle=False, drop_last=True, **loader_args)

    # 4. 设置优化算法，学习率调度器，损失函数r，以及AMP（自动混合精度）
    optimizer = optim.RMSprop(net.parameters(), lr=learning_rate, weight_decay=1e-8, momentum=0.9) #优化算法
    scheduler = optim.lr_scheduler.ReduceLROnPlateau(optimizer, 'max', patience=2) #学习率调度器来最大化Dice度   
    grad_scaler = torch.cuda.amp.GradScaler(enabled=amp)                                                         criterion = nn.CrossEntropyLoss()                  #损失函数
    global_step = 0
    # 5.开始训练
    for epoch in range(1, epochs+1):  #迭代设置的次数
        net.train()                 
        epoch_loss = 0                #记录loss值
        with tqdm(total=n_train, desc=f'Epoch {epoch}/{epochs}', unit='img') as pbar:
            for batch in train_loader:      #对每一批次数据的图像，mask标签
                images = batch['image']     #取出图像
                true_masks = batch['mask']  #取出标签

                assert images.shape[1] == net.n_channels, \
                    f'Network has been defined with {net.n_channels} input channels, ' \
                    f'but loaded images have {images.shape[1]} channels. Please check that ' \
                    'the images are loaded correctly.'

                images = images.to(device=device, dtype=torch.float32)      #指定训练用到gpu装置
                true_masks = true_masks.to(device=device, dtype=torch.long)

                with torch.cuda.amp.autocast(enabled=amp):
                    masks_pred = net(images)  
                    loss = criterion(masks_pred, true_masks) \               #损失计算
                           + dice_loss(F.softmax(masks_pred, dim=1).float(),
                                       F.one_hot(true_masks, net.n_classes).permute(0, 3, 1, 2).float(),
                                       multiclass=True)

                optimizer.zero_grad(set_to_none=True)
                grad_scaler.scale(loss).backward()  #根据loss反向传播，
                grad_scaler.step(optimizer)
                grad_scaler.update()

                pbar.update(images.shape[0])   #编辑一些输出的信息，方便用户获取训练的信息
                global_step += 1          
                epoch_loss += loss.item() 
               
                pbar.set_postfix(**{'loss (batch)': loss.item()})

                # Evaluation round
                division_step = (n_train // (10 * batch_size))
                if division_step > 0:
                    if global_step % division_step == 0:
                        val_score = evaluate(net, val_loader, device)
                        scheduler.step(val_score)
        if save_checkpoint:
            Path(dir_checkpoint).mkdir(parents=True, exist_ok=True)
            torch.save(net.state_dict(), str(dir_checkpoint / 'checkpoint_epoch{}.pth'.format(epoch)))
            logging.info(f'Checkpoint {epoch} saved!')
            
            def get_args():   #读取用户设置的参数
    parser = argparse.ArgumentParser(description='Train the UNet on images and target masks')
    parser.add_argument('--epochs', '-e', metavar='E', type=int, default=5, help='Number of epochs')#迭代次数
    parser.add_argument('--batch-size', '-b', dest='batch_size', metavar='B', type=int, default=1, help='Batch size')                                                                                  #批量大小
    parser.add_argument('--learning-rate', '-l', metavar='LR', type=float, default=1e-5,
                        help='Learning rate', dest='lr')                                            #学习率
    parser.add_argument('--load', '-f', type=str, default=False, help='Load model from a .pth file')
    parser.add_argument('--scale', '-s', type=float, default=0.5, 
                        help='Downscaling factor of the images')                               #放缩图像的大小
    
    parser.add_argument('--validation', '-v', dest='val', type=float, default=10.0,
                        help='Percent of the data that is used as validation (0-100)')            #验证集比例
    parser.add_argument('--amp', action='store_true', default=False, help='Use mixed precision') 
    parser.add_argument('--bilinear', action='store_true', default=False, help='Use bilinear upsampling')
    parser.add_argument('--classes', '-c', type=int, default=2, help='Number of classes')          #类别数

    return parser.parse_args()


if __name__ == '__main__':

       dir_img = Path('photos\photos_original')      #原图像数据集目录
       dir_mask = Path('photos\masks')               #mask标签图像数据目录
       dir_checkpoint = Path('checkpoint_new')       #checkpoint训练得到的参数保存目录
           
       args = get_args()

       logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
       device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        logging.info(f'Using device {device}')

    
    #n_channel 彩色图片rgb三个通道 
    # n_classes 指所有像素点被分为的类别数
    net = UNet(n_channels=3, n_classes=args.classes, bilinear=args.bilinear)
    
    logging.info(f'Network:\n'
                 f'\t{net.n_channels} input channels\n'
                 f'\t{net.n_classes} output channels (classes)\n'
                 f'\t{"Bilinear" if net.bilinear else "Transposed conv"} upscaling')

    if args.load:
        net.load_state_dict(torch.load(args.load, map_location=device))
        logging.info(f'Model loaded from {args.load}')

    net.to(device=device)
    # print(summary(net, (3, 800, 600), 4, "cuda"))
    try:
        train_net(net=net,
                  epochs=args.epochs,
                  batch_size=args.batch_size,
                  learning_rate=args.lr,
                  device=device,
                  img_scale=args.scale,
                  val_percent=args.val / 100,
                  amp=args.amp)
    except KeyboardInterrupt:
        torch.save(net.state_dict(), 'INTERRUPTED.pth')
        logging.info('Saved interrupt')
        raise

```

```
模型结构文件路径：\car_project\unet_torch\unet
模型结构快定义文件路径：\car_project\unet_torch\unet
数据装载文件路径：\car_project\unet_torch\utils
```

  2.在本地电脑试运行一下训练模型的代码：打开command终端，进入虚拟环境，cd至train.py目录下，输入命令(例)：

```
python train.py -l 0.0002 -s 0.5 -e 2 -c 3 -b 4 -v 25    # l:学习率 s:放缩比例 e:迭代次数 c:类别数 b:批量大小 v:验证集占比（除以100后为比率） 更多参数可以参考unet_torch项目中的markdown文档
```

![image-20220823100017233](picture\image-20220823100017233.png)

  输出“checkpoint xx saved！”即说明训练成功。在我们指定的checkpoint路径下可以找到训练好checkpoint模型参数文件

  3.简单测试一下已经训练好的模型的预测功能，在command中输入例如：

````
python predict_custom.py -m checkpoint_new\checkpoint_epoch29.pth -i test\photo_19700101_000723.jpg -v -n
#-m:checkpoint文件路径  -i:需要预测的图片路径  -v:预测结果图片显示  -n:预测结果不再存储...
#更多参数可以参考该项目下的readme.md
````

结果：

![image-20220823115854084](picture\image-20220823115854084.png)

                 从左到右，依次为：原图，road类（黄），grass类（黄），background类（黄）

## 5.在算力服务平台训练模型

  1.本次实验我们在中科曙光的曙光智算平台训练我们的模型。在平台注册账号后，购买或者领取后，直接进入控制台。

  <img src="picture\image-20220823141415951.png" alt="image-20220823141415951" style="zoom:67%;" />



  2.配置环境

<img src="picture\image-20220823141640985.png" alt="image-20220823141640985" style="zoom:80%;" />

  页面顶栏中命令行可以打开linux终端，E-File打开可视化文件管理

  <img src="picture\image-20220823145113434.png" alt="image-20220823145113434" style="zoom:67%;" />

   可视化编辑文件夹的功能非常方便，可以专门把接下来的安装包放入其中

（1）安装anaconda：方法一：终端执行命令

```
wget https://repo.anaconda.com/archive/Anaconda3-2022.05-Linux-x86_64.sh     #直从官网下载安装包
```

方法二：将.sh安装包下载到本地，再利用E-File 的上传功能把安装包上传到指定目录下。![image-20220823143111544](picture\image-20220823143111544.png)

（2）安装软件，并将anaconda添加到环境中

```
chmod +x ./Anaconda3-2022.05-Linux-x86_64.sh #随后运行
```

安装完成后运行命令：

```
source ~/.bashrc
```

与在windows中同样的，安装完成后我们需要检查一次anaconda是否配置到了系统环境中。

在终端中输入：

```
conda
```

回车，若报错则需要手动将anaconda添加到环境中。

添加方法如下：在终端中输入

```
nano ~/.bashrc
```

进入页面：

<img src="picture\image-20220823154548326.png" alt="image-20220823154548326" style="zoom:67%;" />

在其中的某个空白行上，填入anaconda的bin文件夹的路径：

```
export PATH=/public/home/acddp272rz/anaconda3/bin:$PATH
```

添加的格式与路径上下对应如图：

![image-20220823154712496](picture\image-20220823154712496.png)

添加完成后按 “ctrl+x”退出，选择确认保存即可。

（3）配置虚拟环境

       ① 进入虚拟环境，值得注意的一点是，进入环境时可能会报错：

<img src="picture\image-20220823162803122.png" alt="image-20220823162803122" style="zoom:67%;" />

出现这种情况，运行：

```
source activate  
```

再重新进入环境，即可解决问题。

    ②安装依赖的包，两种方法：

  （1）可以自行查看曙光平台提供的帮助文档：

<img src="picture\image-20220823172017525.png" alt="image-20220823172017525" style="zoom:67%;" />

<img src="picture\image-20220823174139251.png" alt="image-20220823174139251" style="zoom:67%;" />

按照文档一步一步按照pytorch，以及其依赖包。安装完成后，再根据我们的训练项目py文件中所引用的包，以及运行后出现的“不存在xx包”的报错信息，用pip install命令，一步一步吧所需的包安装进环境里。

  （2）将之前配置windows的packages.txt文件上传到主机里，同样地运行：

```
pip install -r packages.txt -i https://pypi.tuna.tsinghua.edu.cn/simple
```

 就把所以所需的包安装上了，当然可能会有一些训练模型不需要的包，但这样更省时省力一些。



  3.在E-File中新建项目文件夹，将之前用的Unet训练项目文件夹上传到该目录下：

  **上传之前有一点需要注意，windows中路径用的是右斜杠（“\”），linux路径用的是左斜杠（“/”），在上传之前需要把python程序中指定目录的部分的右斜杠改为左斜杠：**



<img src="picture\image-20220823161351506.png" alt="image-20220823161351506" style="zoom:67%;" />

如果觉得普通上传太慢的话也可以点击快传，下载相应软件后就可以快速把文件夹上传到E-File中了。

  

4.终端输入命令，申请算力资源：

```
salloc -p kshdtest -N 1 --gres=dcu:4 --cpus-per-task=16
# -p:申请的队列名 -N:申请的节点数 -dcu:申请的dcu数量 -cpu:申请的cpu数量
```

队列名的查看如下图：

<img src="picture\image-20220823165827213.png" alt="image-20220823165827213" style="zoom:67%;" />

  申请完成后（如下）：

<img src="picture\image-20220823165922736.png" alt="image-20220823165922736" style="zoom:80%;" />

  输入命令：

  ````
squeue  #获取申请到的主机名，下图红框标注即是
  ````

<img src="picture\image-20220823170146938.png" alt="image-20220823170146938" style="zoom:67%;" />

  登录申请到的主机：

```
ssh j17r2n02    #ssh + 主机名
```

  

5.移动到项目的目录下后，直接运行训练模型的程序：

 ````
python train.py -e 30 -b 16 -l 0.0001 -s 0.5 -c 3    # l:学习率 s:放缩比例 e:迭代次数 c:类别数 b:批量大小 v:验证集占比（除以100后为比率） 更多参数可以参考unet_torch项目中的markdown文档
 ````

<img src="picture\image-20220823192539964.png" alt="image-20220823192539964" style="zoom:80%;" />



  6.训练完成，去到对应目录下载参数文件到本地即可。

<img src="picture\image-20220823205012345.png" alt="image-20220823205012345" style="zoom:67%;" />



## 6.鸟瞰图变换

  1.相机成像原理与标定：对物理世界的任一点，推断它在照片上的位置，我们需要以下步骤：

     ①世界坐标系→相机坐标系：由相机位置决定
    
     ② 相机坐标系→图像物理坐标系：按成像位置距离小孔之比，等比例放缩照片平面的x,y轴
    
     ③图像物理坐标系→图像像素坐标系 ：取图像某边角像素点为原点，根据像素点与物理长度之比和像素中心点位置的像素坐标，通过线性变换，计算得到某一点的像素点坐标

   将以上数学计算全部整理为矩阵计算的形式，方便代码处理。

   视角位置影响来源于①（外参），畸变影响来源于②③（内参）。

   在训练完Unet网络后，我们可以对摄像头拍摄来的每一张照片做语义分割，分类出每一个属于道路的像素点，但仅有这个分类好的照片，我们仍不能精确的为我们的小车规划路径，因为我们的道路照片是扭曲且倾斜的，我们需要一个平面的鸟瞰图才能规划好小车的路径，而有了分类出道路的照片后，我们要做的，就是将上述过程反过来推演，通过照片上道路像素点的位置，找到道路像素点在真实物理世界中的位置（或者说在一个平面的类似地图的物理模型中的位置）。

   2.matlab成像实验：

<img src="picture\image-20220824094537832.png" alt="image-20220824094537832" style="zoom:80%;" />

  <img src="picture\image-20220824094710714.png" alt="image-20220824094710714" style="zoom: 50%;" />

                     （上图为去畸变后，下图为去畸变前）

<img src="picture\image-20220824094745959.png" alt="image-20220824094745959" style="zoom:50%;" /> 



3.python成像实验

  （1）内参方面——表格去畸变案例，以及过程分析：

    ①设置客观存在的物点
    
    ②对一张图片进行扫描，检测期盘格角点
    
    ③将物点与角点一一对应，完成去畸变（左为去畸变前，右为对棋盘去畸变后）：

  <img src="picture\image-20220824105231262.png" alt="image-20220824105231262" style="zoom:80%;" />

    ④存储K D r t等值参数（畸变参数，旋转参数，平移参数）

（2）外参方面——棋盘的鸟瞰图变换实验：

      （作图为变换前，右图为鸟瞰变换后）<img src="picture\image-20220824111352597.png" alt="image-20220824111352597" style="zoom:80%;" />

   原棋盘图片的四角点是由函数自行搜索得到，鸟瞰图变换后生成的棋盘的四角像素坐标位置可以自行指定。

   矫正完成后，得到矫正参数文件，在之后的小车中，因为我们的小车摄像头硬件，以及小车的摆放位置是固定的，直接调用我们的矫正参数就可以了.

## 7.路径规划与路径跟踪算法

```
#PATH:\car_project\Astar
#PATH:\car_project\stanley
```



 1.在对像素已经打上标签的图片做鸟瞰图变换之后，我们就相当于得到了一张张“地图”，这些“地图”明确标好了那些是路，哪些地方不可以走。下一步，就是要根据这些“地图”，通过算法，计算出从出发点到终点的路径。

（1）广度优先算法：

从初始节点开始，先逐一探索初始节点的周边第一层节点，检查目标节点是否在这些节点中，若没有，再将所有第一层的节点逐一扩展，得到第二层节点，并逐一检查第二层节点中是否包含目标节点。若没有，再逐一扩展第二层的所有节点……，如此依次扩展，检查下去，直到发现目标节点为止。

<img src="picture\image-20220824145015437.png" alt="image-20220824145015437" style="zoom: 67%;" />



（2）迪克斯特拉（Dijkstra）算法：

从下一步可能要走的点中取出距离起点最小路径的点，走这个点，并以这个点为桥梁更新，求出下一个距离起点最小路径的点，直至到达终点，这样就找到了从起点到终点最近的那条路。

<img src="picture\image-20220824144941891.png" alt="image-20220824144941891" style="zoom: 67%;" />

（3）贪心算法：

只考虑，局部状态下，哪一步最能缩短自己位置与终点的距离，便执行哪一步，直到到达终点。

<img src="picture\image-20220824150448895.png" alt="image-20220824150448895" style="zoom:67%;" />

（4）A*算法：

A*算法在上面那些算法的基础上，引入了启发式搜索，实际上是综合上面这些算法的特点于一身的。

A*算法通过一个函数，对每一个节点做代价评估，区分每个节点的优先级：

                                                        f(n)  =  g(n)  +  h(n)

其中：

①f(n)是节点n的综合优先级。当我们选择下一个要遍历的节点时，我们总会选取综合优先级最高（值最小）的节点。

②g(n) 是节点n距离起点的代价。

③h(n)是节点n距离终点的预计代价，这也就是A*算法的启发函数。

A*算法在运算过程中，每次从优先队列中选取f(n)值最小（优先级最高）的节点作为下一个待遍历的节点，另外可以发现，当启发函数h(n)  =  0 时，A *算法实际上变为了 Dijkstra’s Algorithm 算法。

![image-20220824151902402](picture\image-20220824151902402.png)

2.python 的A*算法实验：

````
PATH:\car_project\Astar   #该小项目的目录
````

<img src="picture\image-20220824154344047.png" alt="image-20220824154344047" style="zoom:80%;" />

3.路径跟踪与Pure Pursuit算法：

    在我们的利用路径规划算法找到一条可行的道路后，我们就需要确保如何才能让小车按既定的路线去行驶。我们已经可以控制小车自由的直行，转弯，停止等，而要想根据路线精确控制小车运动，这就需要我们对小车及其运动建模并研究了。

  （1）阿克曼转向几何模型：

          理想的阿克曼转向，汽车在转弯的时候，两个轮胎的中轴指向同一个原点（如下），有了这个保证，我们可以进一步简化小车运动的模型为二维自行车模型。

<img src="picture\v2-6df5463a76c7eaca5f04ccc461274c55_r.jpg" alt="v2-6df5463a76c7eaca5f04ccc461274c55_r" style="zoom:80%;" /> 

   （2）二维自行车模型：

        ①车辆只有前、后两个车轮；
    
        ②只考虑二维平面的运动，不考虑垂直方向的运功；
    
        ③车辆低速运动，此时滑移角可以忽略不计。

   （3）基于上面的模型，我们可以通过下面的计算得到小车的转角关于时间，小车轴距，以及出发点到下一个目标点的函数：

                                                    <img src="picture\image-20220824185728931.png" alt="image-20220824185728931" style="zoom: 50%;" /> 
    
                                     L：小车轴距            σ：小车某一时刻的转角         R：给定转角后，小车后路所遵循的圆的半径
    
                                                                           而我们目标即为求出σ，或者求出R（目前只有L已知）
    
                                                 <img src="picture\image-20220824190718821.png" alt="image-20220824190718821" style="zoom:50%;" /> 

<img src="picture\image-20220824191015208.png" alt="image-20220824191015208"  />

  <img src="picture\image-20220824190942210.png" alt="image-20220824190942210"  />

    最终得到一个转角σ关于车轴，尾轴到终点距离，以及时间的一个函数，便能实时求出小车需要的转动角度。



4.python Pure Pursuit模拟小车路径跟踪

```
PATH: .\car_project\stanley    #该小项目目录
```



<img src="picture\image-20220824182122822.png" alt="image-20220824182122822" style="zoom: 80%;" />



## 8.元器件：测速计和IMU

  1.测速计:

    本实验用到的是透光式测速传感器，主要由三个组成部分，分别是带孔的回盘、光源和光电管。圆盘随被测轴旋转时，光线只能通过因孔或缺口照射到光电管上。光电管被照射时，其反向电阻很低，于是输出一个电脉冲信号。光源被圆盘遮住时，光电管反向电阻很大，输出端就没有信号输出。这样，根据圆盘上的孔数或缺口数，即可测出被测轴的转速。
    
    VCC——接5V电源          GND——接地             OUT——信号输出口，接在任意io脚，代码声明

  2.IMU:

   VCC——接3.3V电源        GND——接地            int——激活口，接在io脚即可         SDA——数据传输口，接主板SDA口       SCL——计时用，接小车主板SDA口

<img src="picture\image-20220825143943365.png" alt="image-20220825143943365" style="zoom:80%;" />

   上电后可以通过串口监听小车主板，输出的X,Y,Z为模块的位置初值，计算初值需要一定时间，此时不能移动小车或模块。

    IMU可以测量车辆运动时的角速度和加速度，根据时间可以计算出小车的位移和运动角度，但计算位移时容易误差过大，故多用来计算小车角度(一次积分计算)。在工作时IMU模块容易受到周围强磁影响，需要放置于与其他模块有一定距离的地方，且工作时不可随意碰动，需要水平固定在小车上。

   3.调节实验

（1）光电速度传感器直接读取的是舵机的转速，我们需要一个计算公式将转身转化为小车实际的行驶速度，这个公式写在小车主板的程序中，需要根据每个小车不同的安装情况自行计算。

```c
#PATH:

if(millis() - old_millis >= 500){
    car_speed = (count / 18.0 / 21 * 6.2 * 3.14 * 1000 / 500)* 0.75;
    count = 0;
    old_millis = millis();
  }
```



（2）可以用python发送请求获取小车的车速（如下）。

   ```python
import requests
import time
import numpy as np

r = requests.post('http://192.168.4.1/motor_control?speed=100')
time.sleep(5)
r = requests.get('http://192.168.4.1/read_speed')
print(r.text)
time.sleep(3)
r = requests.post('http://192.168.4.1/motor_control?speed=0')
   ```



（3）启动小车电源后，reset主板，再待IMU模块重新矫正完毕，在小车不动的情况下，运行以上代码，获取小车转动角度值，以此来检验一下IMU工作是否稳定，若输出为一个接近0的值，说明IMU正常工作。再试试手动将小车转动90度，再次运行上述代码，若输出值接近90或者270度，说明IMU正常工作。

````python
import requests
import time

r = requests.get('http://192.168.4.1/read_angle')
print(r.text)
````



## 9.无人驾驶

```
#PATH:\car_project\final
```

  以上的准备工作，一步一步规划好了我们无人驾驶的整个过程。而接下来我们可以通过简单分析无人驾驶的最终实现代码，更深入的了解整个过程。

```python
#PATH:\car_project\final\final.py

import cv2
import numpy as np
from urllib.request import urlopen
import threading
import time
import torch
from PIL import Image
import matplotlib.pyplot as plt

from pure_pursuit.utils import car_controllor, VehicleState, pure_pursuit_control
from seg import unet_predict
from seg.unet import UNet
from path_plan.utils import find_path

def read_ip_camera(stream):

    global url
    global img
    global bts
    global is_exit

    count = 0
    while not is_exit:
        try:
            bts += stream.read(CAMERA_BUFFRER_SIZE)
            jpghead=bts.find(b'\xff\xd8')
            jpgend=bts.find(b'\xff\xd9')
            
            if jpghead > -1 and jpgend > -1:
                print(count)
                count += 1
                jpg = bts[jpghead:jpgend + 2]
                bts = bts[jpgend + 2:]
                img = cv2.imdecode(np.frombuffer(jpg, dtype = np.uint8), cv2.IMREAD_UNCHANGED)

        except Exception as e:
            print("Error:" + str(e))
            bts = b''
            stream = urlopen(url)
            continue

'''
主程序
'''
if __name__ == '__main__':
    #过程图像是否展示(有利于分析过程)
    SHOW_IMAGE = True
    #开头先看摄像头，因为我们小车自动行驶的第一步就是观察前方路况
    url = 'http://192.168.4.5:80/'
    stream = urlopen(url)
    CAMERA_BUFFRER_SIZE = 4096
    bts = b''
    img = None
    is_exit = False

    # 开始 imu 读取线程 保证摄像头读取与其他程序正常进行
    t1 = threading.Thread(target=read_ip_camera, args=(stream, ))
    t1.setDaemon(True)
    t1.start()

    time.sleep(5)
    
    #加载道路分割Unet模型，准备预测图片，模型参数为我们独立训练出的
    net = UNet(n_channels=3, n_classes=3, bilinear=False)
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    net.to(device=device)
    net.load_state_dict(torch.load('seg\checkpoints\checkpoint_epoch1.pth', map_location=device))
    
    if SHOW_IMAGE:
        plt.imshow(img[:, :, [2,1,0]])
        plt.show()

    ctrller = car_controllor()
    
    #循环十次：拍照--处理图片--路径规划--决策--拍照.......
    for i in range(10):
        
        #Unet模型预测照片，给照片每一个像素点打标签
        img_Image = Image.fromarray(img)
        mask = unet_predict.predict_img(net=net,
                            full_img=img_Image,
                            scale_factor=0.5,
                            out_threshold=0.5,
                            device=device)
        mask = unet_predict.mask_to_image(mask)
        
        if SHOW_IMAGE:
            plt.imshow(mask)
            plt.show()
       
        #决策寻路，其中find_path为关键函数，集合了去畸变，鸟瞰变换，图片进一步优化，路径规划于一身
        #用一个try的结构去判断：如果前面没有可行驶的路，那么后退四秒
        try:
            path = find_path(mask, time_limit=5, SHOW_IMAGE=SHOW_IMAGE)
        except:
            ctrller.motor_control(-50)
            time.sleep(4)
            ctrller.motor_control(0)
            continue

        path_array = np.array(path)
        path_array = np.vstack((path_array[:, 1], path_array[:, 0])).T
        path_array[:, 1] = 29 - path_array[:, 1]
        path_array = path_array * 40
        print(path_array)  #输出路径

        if SHOW_IMAGE:
            plt.figure()
            plt.scatter(path_array[:, 0], path_array[:, 1], s=20, c='red', alpha=0.6)
            plt.show()


        '''
        路线跟踪常量
        '''
        Lf = 100.0  # look-ahead distance 前视觉距离
        dt = 1.0  # [s]  
        d = 144.0  # [mm] wheel base of vehicle 车轴长度
        delta_candicate = np.array([-28, -16.2, 0.0, 16.2, 28]) / 180 * np.pi
        delta_cmd = np.array([90, 60, 0, -60, -90], np.int16) + 90

        ctrller.servo_control(90)
        ctrller.motor_control(50)
        state = VehicleState(x=800, y=0, delta=0.0, yaw=0, v=0)

        # 1 px = 40 mm 控制小车按规划的路径行驶
        pure_pursuit_control(state, path_array, ctrller)

    is_exit = True
    t1.join()
```

 以下为某次无人驾驶实验从头到尾产出的过程，以及简单分析。

道路照片

![image-20220826162041408](picture\image-20220826162041408.png)

Unet语义分割后

![image-20220826162101313](picture\image-20220826162101313.png)

去畸变后

![image-20220826162141820](picture\image-20220826162141820.png)

鸟瞰变换后

![image-20220826162212007](picture\image-20220826162212007.png)

grass标签获取，并去除grass的小块区域

![image-20220826162232819](picture\image-20220826162232819.png)

background标签，并除去background的小块区域

![image-20220826162252131](picture\image-20220826162252131.png)

可视化，此时有四类像素，road，grass，background以及空白（鸟瞰变换形成或因属于游离的小区域而被删去）

然后分别对grass和background做扩展，把边缘变得光滑

![image-20220826162305495](picture\image-20220826162305495.png)

像素点缩放，缩放为每个像素点四厘米

同时补全“空白”区域

![image-20220826162320473](picture\image-20220826162320473.png)

去掉与起点无法连接的路

![image-20220826162336952](picture\image-20220826162336952.png)

![image-20220826162354119](picture\image-20220826162354119.png)

选取终点的弧：80cm

![image-20220826162408261](picture\image-20220826162408261.png)

取到的可作为终点的点

![image-20220826162424983](picture\image-20220826162424983.png)

之后就是小车根据路径规划算法，找到对应路线，并通过路径跟踪，沿路线行驶至目标点。



至此，本次无人驾驶项目至此完成。
