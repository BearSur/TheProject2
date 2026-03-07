# TheProject2 - 技术文档

## 一、项目概述
这是一个虚幻引擎5（UE5）第三人称格斗游戏项目，采用现代C++和蓝图混合开发，重点实现了角色战斗系统、AI控制、能力系统等核心游戏机制。

---

## 二、技术栈

### 核心引擎与框架
- **游戏引擎**: Unreal Engine 5.7（最新版本）
- **开发语言**: C++17 + Blueprint（蓝图）
- **构建系统**: Unreal Build Tool (UBT)

### 关键依赖模块
| 模块名 | 用途 | 类别 |
|--------|------|------|
| **Core** | 基础引擎核心功能 | 公开依赖 |
| **CoreUObject** | 对象系统（UOBJECT等） | 公开依赖 |
| **Engine** | 引擎主模块 | 公开依赖 |
| **InputCore** | 基础输入系统 | 公开依赖 |
| **EnhancedInput** | 增强输入系统（IMC + IA） | 公开依赖 |
| **AIModule** | AI基础模块（行为树、黑板等） | 公开依赖 |
| **StateTreeModule** | 状态树系统 | 公开依赖 |
| **GameplayStateTreeModule** | Gameplay与状态树集成 | 公开依赖 |
| **UMG** | UI框架 | 公开依赖 |
| **Slate** | 编辑器UI框架 | 公开依赖 |
| **MotionWarping** | 动作弯曲（脚步同步） | 公开依赖 |
| **GameplayAbilities** | GAS（Gameplay Ability System） | 私有依赖 |
| **GameplayTags** | GAS标签系统 | 私有依赖 |
| **GameplayTasks** | GAS任务系统 | 私有依赖 |

### 架构设计模式
- **Gameplay Ability System (GAS)**: 用于角色属性、能力和效果管理
- **行为树 (Behavior Tree)**: AI决策与状态管理
- **黑板 (Blackboard)**: AI信息共享和状态存储
- **增强输入系统**: 灵活的输入绑定和映射
- **标签系统 (Gameplay Tags)**: 角色状态和事件标记

---

## 三、核心系统架构

### 3.1 角色系统

#### 类关系图
```
ACharacter (虚幻基类)
    ├── APlayerCharacter (玩家角色)
    │   ├── 继承: IAbilitySystemInterface
    │   ├── 包含: UAbilitySystemComponent
    │   ├── 包含: UBaseAttributeSet
    │   ├── 包含: USpringArmComponent (相机臂)
    │   ├── 包含: UCameraComponent (跟随相机)
    │   ├── 包含: UAIPerceptionStimuliSourceComponent (AI感知源)
    │   └── 包含: UStaticMeshComponent (武器网格)
    │
    └── ABaseAICharacter (AI角色)
        ├── 继承: IAbilitySystemInterface
        ├── 包含: UAbilitySystemComponent
        ├── 包含: UBaseAttributeSet
        └── 数据: UBehaviorTree (行为树资源)
```

#### 字符初始化流程
```
1. 角色生成(Constructor)
   └─> 创建组件并初始化配置

2. 被Controller占有 (PossessedBy)
   └─> InitAbilitySystem()
       └─> AbilitySystemComponent->InitAbilityActorInfo()
       └─> GiveDefaultAbilities()
           └─> 遍历CharacterAbilities数组
           └─> 给每个能力赋予GAS Spec

3. 游戏开始(BeginPlay)
   └─> InitializeAttribute()
       └─> 创建DefaultAttributeEffect游戏效果
       └─> 应用GE到自身属性集合
       └─> 初始化Health, MaxHealth, Stamina等属性
```

### 3.2 属性与伤害系统

#### 属性集合 (BaseAttributeSet)
```cpp
// 主属性
- Health (血量)
- MaxHealth (最大血量)
- Stamina (耐力，用于近战/奔跑)
- MaxStamina (最大耐力)
- Poise (韧性值，抗打断能力)

// 元属性(Meta Attributes) - 用于临时计算
- Damage (伤害值)
- Healing (治疗值)
- PoiseDamage (韧性伤害)
```

#### 伤害流程
```
攻击方 -> 应用GameplayEffect(Damage) 
    -> BaseAttributeSet::PostGameplayEffectExecute()
    -> 检测Damage属性修改
    -> 计算: NewHealth = CurrentHealth - Damage
    -> 触发死亡检测
    -> 发送Event.Character.Death事件
```

#### 韧性系统流程
```
攻击方 -> 应用GameplayEffect(PoiseDamage)
    -> BaseAttributeSet::PostGameplayEffectExecute()
    -> 判断: localPoiseDamage > Poise + 100?
    -> YES: 添加"Character.HitState.Heavy"标签 (硬直)
    -> NO:  添加"Character.HitState.Light"标签  (轻微硬直)
```

### 3.3 战斗系统 (CombatSystemComp)

#### 功能模块
```
UCombatSystemComp (战斗系统组件) 
    ├─ 锁定系统 (Target Lock-On)
    ├─ 受击方向判定
    ├─ 翻滚动作 (Roll)
    └─ 动作弯曲 (Motion Warping)
```

#### 3.3.1 目标锁定流程 (TargetLockOn)
```
UCombatSystemComp::TargetLockOn()
    └─> 以角色位置为中心，Radius范围内做球体扫描
        ├─> FCollisionShape::MakeSphere(Radius)
        ├─> 扫描类型: ECC_Pawn + ECC_WorldDynamic
        └─> 获取OverlapResults[]
    
    └─> 遍历扫描结果，查找ULockonPointComp
        ├─> 计算Every LockonPointComp到角色的距离
        ├─> 筛选: IsLockable==true且距离最近的点
        └─> 选出BestComp
    
    └─> 锁定目标
        ├─> LockPoint = BestComp
        ├─> CharacterMovement->bOrientRotationToMovement = false
        └─> CharacterMovement->bUseControllerDesiredRotation = true

Tick每帧执行:
    └─> 若LockPoint有效且IsLockable==true
        └─> 计算LookAtRotation(CharacterLoc → LockPoint)
        └─> 平滑旋转角色面向目标
```

#### 3.3.2 受击方向判定 (CalculateHitDirection)
```
攻击发起者调用: CalculateHitDirection(FVector HitLocation, AActor* TargetActor)

1. 获取目标ASC
   └─> Cast<IAbilitySystemInterface>(TargetActor)->GetAbilitySystemComponent()

2. 检测目标是否有受击状态标签
   └─> if (!HasTag("Character.HitState")) return; // 韧性值过低

3. 坐标变换 - 将攻击点从世界坐标转换为目标的本地坐标
   └─> LocalHitLoc = TargetActor->InverseTransformPosition(HitLocation)
   └─> 含义: X轴=前后, Y轴=左右, Z轴=上下

4. 后背判定
   └─> if (LocalHitLoc.X < -10) → 判定为后背受击

5. 正面四向判定 (使用权重补偿高度)
   └─> WeightedY = Abs(LocalHitLoc.Y) * 2.0f (高宽比补偿)
   └─> if (WeightedY > Abs(LocalHitLoc.Z))
       ├─> YES: 左右受击 (Y > 0 → Left, Y < 0 → Right)
       └─> NO:  上下受击 (Z > 0 → Up, Z < 0 → Down)

6. Tag激活
   └─> 根据目标当前状态(Heavy/Light)+ 方向组成最终Tag
   └─> 例: "Character.HitState.Heavy.Up"
   
7. 发送事件
   └─> SendGameplayEventToActor(TargetActor, "Event.Character.HitReact", Payload)
   └─> 目标的蓝图或GE监听该事件播放对应方向的受击动画
```

#### 3.3.3 翻滚系统 (DoDirectionalRoll)
```
输入: FVector2D InputValue (摇杆/按键输入)

1. 调用GetRollDirection(InputDirection)
   └─> 标准化InputValue方向
   └─> 返回对应的ERollAnimationType
       ├─> Angle < 45°   → Right
       ├─> Angle < 135°  → Front
       ├─> Angle < 225°  → Left
       ├─> Angle < 315°  → Back
       └─> 其他对角线 → FrontLeft/FrontRight/BackLeft/BackRight

2. 从WeaponData获取对应的RollAnimMontage

3. 构建GameplayEvent
   └─> EventTag: "Event.Character.Roll"
   └─> RollDirection: ERollAnimationType

4. 应用MotionWarping(可选)
   └─> 若SkillData.ShouldDoMotionWarping == true
   └─> 计算目标位移 via CalculateMotionWarpingTarget()
```

#### 3.3.4 动作弯曲 (Motion Warping)
```
CalculateMotionWarpingTarget(MaxDistance, TargetLocation, TargetRotation, WarpTargetName)

1. 计算当前角色到目标位置的距离
2. 若实际距离 > MaxDistance
   └─> 限制吸附距离为MaxDistance，避免无限远距离吸附
3. 返回FMotionWarpingTarget
   └─> 在蒙太奇播放时同步脚步位置
```

### 3.4 AI系统

#### AI类关系
```
ABaseAIController (AI控制器)
    ├─ 包含: UBehaviorTreeComponent (运行行为树)
    ├─ 包含: UBlackboardComponent (黑板 - 信息共享)
    ├─ 包含: UAIPerceptionComponent (AI感知)
    └─ 包含: UAISenseConfig_Sight (视觉感知配置)
        ├─ SightRadius: 1500cm (视野距离)
        ├─ LoseSightRadius: 2000cm (失去视野距离)
        └─ PeripheralVisionAngleDegrees: 90° (周边视角)

ABaseAICharacter (AI角色)
    ├─ 继承自: ACharacter + IAbilitySystemInterface
    └─ 数据: BehaviorTree (关联的行为树资源)
```

#### AI初始化流程
```
1. AI被Controller占有 (BaseAIController::OnPossess)
   
2. 初始化黑板
   └─> BlackboardComp->InitializeBlackboard(BehaviorTree->BlackboardAsset)
   
3. 运行行为树
   └─> RunBehaviorTree(AICharacter->BehaviorTree)
   └─> BehaviorTreeComp开始执行节点任务

4. 设置GAS (与玩家角色相同)
   └─> InitAbilitySystem()
   └─> GiveDefaultAbilities()
```

#### AI感知与目标检测流程
```
1. UAIPerceptionComponent 监听周围世界

2. 目标进入感知范围 (OnTargetPerceptionUpdated)
   └─> BaseAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
   
3. 若成功感知 (WasSuccessfullySensed())
   └─> BlackboardComp->SetValueAsObject("Target", Actor)
   └─> 行为树读取Target黑板值，做出响应决策
   
4. 感知丧失
   └─> 清除Target黑板值
```

---

## 四、数据资源架构

### 4.1 武器数据 (WeaponData)
```cpp
UWeaponData : UPrimaryDataAsset
    ├─ WeaponMesh (UStaticMesh) - 武器网格模型
    ├─ LightAttackChain (TArray<USkillData*>) - 轻攻击链
    ├─ HeavyAttackChain (TArray<USkillData*>) - 重攻击链
    ├─ WeaponAnimationType (EWeaponAnimationType) - 武器类型
    │   ├─ Sword (剑)
    │   └─ LongSword (长剑)
    ├─ TagsToAnimationMap (TMap<FGameplayTag, UAnimMontage*>) 
    │   └─ 根据GameplayTag查找对应动画
    ├─ RollAnimationMap (TMap<ERollAnimationType, UAnimMontage*>)
    │   └─ 八向翻滚动画映射
    └─ WeaponAnimInstanceClass (TSubclassOf<UAnimInstance>)
        └─ 武器的自定义动画蓝图类
```

### 4.2 技能数据 (SkillData)
```cpp
USkillData : UPrimaryDataAsset
    ├─ SkillAnim (UAnimMontage) - 技能动画
    ├─ Damage (TArray<float>) 
    │   └─ 保存技能的多段伤害值
    │   └─ 攻击的第N段读取第N个元素
    ├─ Poise (TArray<float>)
    │   └─ 多段韧性伤害值
    ├─ ShouldDoMotionWarping (bool) - 是否启用动作弯曲
    └─ MaxMotionWarpingDistance (float) - 最大吸附距离(默认200cm)
```

### 4.3 碰撞箱数据 (HitboxData)
```cpp
UHitboxData : UPrimaryDataAsset
    ├─ StartSocketName (FName) - 碰撞箱起始点(骨骼套接字)
    ├─ EndSocketName (FName) - 碰撞箱结束点(骨骼套接字)
    ├─ HitObjectType (TArray<EObjectTypeQuery>) - 检测对象类型
    └─ HitboxRadius (float) - 碰撞球体半径
```

### 4.4 锁定点组件 (LockonPointComp)
```cpp
ULockonPointComp : USceneComponent
    └─ IsLockable (bool) - 是否可被锁定(默认true)
```

---

## 五、输入系统流程

### 5.1 增强输入绑定流程
```
BasePlayerController::SetupInputComponent()
    └─> 获取UEnhancedInputLocalPlayerSubsystem
    └─> 遍历DefaultMappingContexts[]
    └─> 添加所有IMC到玩家输入系统

APlayerCharacter::SetupPlayerInputComponent()
    └─> 绑定输入动作到函数:
        ├─ JumpAction 
        │   ├─> Started  → ACharacter::Jump()
        │   └─> Completed → ACharacter::StopJumping()
        ├─ MoveAction  → APlayerCharacter::Move()
        ├─ LookAction  → APlayerCharacter::Look()
        └─ MouseLookAction → APlayerCharacter::Look()
```

### 5.2 移动输入处理
```
Move(FInputActionValue Value)
    └─> 提取FVector2D MovementVector
    └─> 调用DoMove(Right, Forward)
    └─> 添加运动力到角色
```

---

## 六、主要功能流程示例

### 6.1 攻击与伤害完整流程
```
玩家按下攻击键
    ↓
PlayerCharacter的GAS激活攻击能力
    ↓
执行SkillData中对应的攻击动画
    ↓
动画中调用碰撞检测 (via Notifies或自定义逻辑)
    └─> 在StartSocket到EndSocket间进行球体扫描
    └─> 找到所有被击中的敌人
    ↓
对每个被击中的敌人:
    1. 应用伤害 GameplayEffect
       └─> TargetASC->ApplyGameplayEffectSpecToSelf(DamageGE)
    
    2. 在BaseAttributeSet::PostGameplayEffectExecute()中:
       └─> Health -= Damage
       └─> 检测死亡事件
    
    3. 应用韧性伤害 GameplayEffect
       └─> 添加"Character.HitState.Heavy/Light"标签
    
    4. 计算受击方向
       └─> CombatSystemComp->CalculateHitDirection(HitLocation, Target)
       └─> 确定方向标签 (Up/Down/Left/Right/Back)
       └─> 发送"Event.Character.HitReact"事件
    
    5. 目标播放对应方向的受击蒙太奇
       └─> UAnimMontage 由方向标签从TagsToAnimationMap查询
```

### 6.2 AI感知与追击流程
```
AI角色在场景中巡逻(运行BehaviorTree)
    ↓
玩家进入AI视野范围(1500cm)
    ↓
UIPerceptionComponent检测到玩家
    ↓
BaseAIController::OnTargetDetected()触发
    ├─> Stimulus.WasSuccessfullySensed() == true
    └─> BlackboardComp->SetValueAsObject("Target", PlayerCharacter)
    ↓
BehaviorTree读取"Target"黑板值
    └─> 切换到追击/战斗状态
    └─> 使用AIMovement向玩家位置移动
    └─> 执行攻击或躲避任务
    ↓
玩家离开AI视野(> 2000cm)
    ↓
OnTargetDetected()再次触发
    ├─> Stimulus.WasSuccessfullySensed() == false
    └─> 清除Target黑板值或设为null
    ↓
BehavierTree返回巡逻或空闲状态
```

### 6.3 锁定与翻滚流程
```
玩家按下锁定键
    ↓
UCombatSystemComp::TargetLockOn()
    ├─> 1000cm半径内球体扫描
    ├─> 查找所有拥有ULockonPointComp的Actor
    ├─> 筛选最近的可锁定点(IsLockable=true)
    └─> LockPoint = SelectedComponent
    ↓
TickComponent每帧:
    ├─> 获取LockPoint世界位置
    ├─> 计算从角色到LockPoint的LookAtRotation
    └─> 逐帧旋转角色面向锁定点
    
玩家输入翻滚方向 (摇杆/方向键)
    ↓
CombatSystemComp::DoDirectionalRoll(FVector2D Input)
    ├─> GetRollDirection(Input) → ERollAnimationType
    ├─> WeaponData->RollAnimationMap[RollType]  → UAnimMontage
    └─> 发送"Event.Character.Roll"事件
    ↓
PlayerCharacter蒙太奇开始播放(八向翻滚)
    ├─> Motion Warping同步脚步(距离限制在MaxMotionWarpingDistance)
    └─> 播放结束后恢复到行走/待机状态
    ↓
若还在锁定状态:
    └─> 继续面向LockPoint
```

---

## 七、文件结构总览

```
Source/TheProject2/
├── Public/
│   ├── PlayerCharacter.h          - 玩家角色主类
│   ├── BaseAICharacter.h          - AI角色基类
│   ├── BaseAIController.h         - AI控制器
│   ├── BasePlayerController.h     - 玩家控制器
│   ├── CombatSystemComp.h         - 战斗系统组件
│   ├── BaseAttributeSet.h         - GAS属性集合
│   ├── LockonPointComp.h          - 锁定点组件
│   ├── WeaponData.h               - 武器数据资源
│   ├── SkillData.h                - 技能数据资源
│   └── HitboxData.h               - 碰撞箱数据
│
├── Private/
│   ├── PlayerCharacter.cpp        - 玩家逻辑实现
│   ├── BaseAICharacter.cpp        - AI角色实现
│   ├── BaseAIController.cpp       - AI控制器实现
│   ├── BasePlayerController.cpp   - 玩家控制器实现
│   ├── CombatSystemComp.cpp       - 战斗系统实现
│   ├── BaseAttributeSet.cpp       - GAS属性逻辑
│   ├── LockonPointComp.cpp        - 锁定点实现
│   ├── WeaponData.cpp             - 空实现
│   ├── SkillData.cpp              - 空实现
│   └── HitboxData.cpp             - 空实现
│
├── TheProject2.h                  - 模块主头文件
├── TheProject2.cpp                - 模块实现
├── BaseGameMode.h/cpp             - 游戏模式基类
├── CombatSystem.h/cpp             - 战斗系统接口
├── CustomEnum.h                   - 自定义枚举
└── TheProject2.Build.cs           - 构建配置
```

---

## 八、设计要点与最佳实践

### 8.1 GAS设计特点
- **模块化**: 属性、效果、能力完全解耦
- **事件驱动**: 通过GameplayTag和GameplayEvent实现系统间通信
- **网络就绪**: 内置网络同步支持
- **蓝图友好**: 大量UFUNCTION和UPROPERTY注解便于编辑器配置

### 8.2 战斗系统设计特点
- **方向感知**: 受击方向判定使用权重补偿，更贴合游戏感受
- **动作同步**: Motion Warping确保攻击动作与实际移动同步
- **灵活数据驱动**: WeaponData和SkillData可由非程序员在编辑器配置

### 8.3 AI系统设计特点
- **感知驱动**: AI通过AIPercepton感知世界，而非直接访问数据
- **行为树**: 高效的决策树结构，易于扩展新行为
- **共享状态**: Blackboard提供AI间的信息交换机制

### 8.4 输入系统设计特点
- **增强输入**: 支持多平台输入(键鼠、手柄、触屏)
- **IMC映射**: 便于动态修改按键绑定
- **响应式**: 快速的输入→响应管线

---

## 九、扩展建议

### 9.1 后续功能扩展方向
1. **多武器系统**: 扩展WeaponData支持更多武器特效
2. **技能组合**: 实现技能链式触发系统
3. **网络同步**: 配置GAS的网络复制规则实现多人PVP
4. **UI系统**: 集成UMG实现血条、能力条、锁定UI等
5. **敌人类型**: 继承BaseAICharacter创建不同敌人种类

### 9.2 性能优化方向
1. **碰撞检测**: 考虑使用多线程扫描或碰撞体池化
2. **动画蒙太奇**: 预加载常用动画，支持异步加载
3. **视野剔除**: 超出相机范围外AI休眠
4. **对象池**: 技能效果和伤害数字文本对象池

---

## 十、关键代码快速查询

| 功能 | 文件 | 函数 |
|------|------|------|
| 角色初始化 | PlayerCharacter.cpp | BeginPlay(), PossessedBy() |
| 伤害计算 | BaseAttributeSet.cpp | PostGameplayEffectExecute() |
| 目标锁定 | CombatSystemComp.cpp | TargetLockOn(), TickComponent() |
| 受击判定 | CombatSystemComp.cpp | CalculateHitDirection() |
| AI感知 | BaseAIController.cpp | OnTargetDetected() |
| 输入处理 | PlayerCharacter.cpp | SetupPlayerInputComponent() |
| AI运行 | BaseAIController.cpp | OnPossess() |
| 翻滚系统 | CombatSystemComp.cpp | DoDirectionalRoll(), GetRollDirection() |

---

**文档最后更新**: 2026年3月7日  
**项目版本**: UE 5.7  
**开发状态**: 活跃开发中
