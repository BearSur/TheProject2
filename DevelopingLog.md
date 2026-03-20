## 技术栈

GAS系统（Gameplay ability,effect,attribute),ai行为树(decorator, service,task),动画蓝图（linked anim graph,linked anim layer, animnotify state, anim montage）。

## 项目介绍    

这是一个基于GAS系统的数据驱动的arpg战斗框架项目，使用actor component思想，增强代码复用性，支持轻重连击，防御，翻滚，锁定敌人操作，敌人会根据攻击的方向，强度等播放不同动画，除了常规的血量外，我还引入了韧性系统，攻击韧性过高的敌人不会造成硬直，韧性较低的敌人则会有硬直。在动画上，我使用了linked anim layer，和linked anim graph以提高复用率。 对动作蓝图和GAS有较深研究，但是对运镜，视听效果熟悉度不高。

## 机制流程

**组件化**：战斗系统逻辑集中在组件中，方便复用。

**韧性机制**：比大小，攻击韧性大就有受击动画，否则打不动，根据大的幅度也会有不同程度的动画。

**数据驱动**：主要集中在武器和角色招式上，使用DataAsset来存储武器数据，包括武器模型，轻重连招数据资产（后续介绍），武器对应的方向性受击，翻滚动画，还有对应动画蓝图（使用linked anim layer）。   对于每个招式，同样采用data asset，主要存储对应招式的动画蒙太奇，和伤害，韧性值；对于动画扭曲的位移时机，hitbox的生成时机等，都在蒙太奇中通过notifystate实现。

**招式的动画蒙太奇**：蒙太奇里会存储招式的hitbox生成时机，动画扭曲的位移时机，可取消时机，这是为了方便直观的配置。

**连招实现原理**：只有轻重连招两种连招树，当互相切换时前一个连招树的计数会清零，连招流程：有两个gameplay ability，在战斗系统组件中也有两个连击计数器，分别对应轻重攻击，进行轻攻击时，会读取轻的连击计数器，播放对应动画，同时把重的连击计数器清零，反过来也是一样。

**攻击实现**：通过notify state在攻击时调用combat comp的hit detection函数。函数里会通过socket获取生成hitbox的位置，然后坐一次碰撞检测，检测打到东西是否有ability system comp，有则发送。

**避免同帧多次击中**:维护hitactor array，攻击结束清除一次。

防御：防御会提升韧性，按下防御键时通过gameplay effect来增加韧性值，在解除防御后会移除gameplay effect。

gameplay tag**互斥管理**：

攻击：在翻滚，受击动画时无法攻击，采用activation blocked tag，当有翻滚或者受击对应的tag时不会触发。

**翻滚实现原理**：

根据输入的vector2d计算出翻滚方向，然后通过FGameplayAbilityTargetData 传入。

**受击方向计算**：

**AI系统**：

基于behavior tree，使用了decorator和service结点。

## 可拓展

更多情况的攻击动画 跳跃时，......

防御方向化，当前是在全身防御

hitbox只有一种，即绑定武器模型的hitbox

逻辑迁移：把逻辑迁移到controller，gameinstance中
