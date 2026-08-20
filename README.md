# 地形模拟
记录实现 地形 海洋 树 草 湖泊 云的项目,开始时间2026/8/9
当前效果:
<img width="1370" height="813" alt="2e452b485b3d38e124d44b131728a496" src="https://github.com/user-attachments/assets/83ff3b64-2550-4994-8046-98b2bb54d957" />

历史进展:
2026/8/19
原谅我从这个时候才开始记录的awa 
实现了侵蚀模拟+噪声生成地形,成功在gpu跑通侵蚀模拟,在今天增加了compute shader
<img width="804" height="647" alt="9f50686d8c9aa14b3048775af5872243" src="https://github.com/user-attachments/assets/bb5a5fb2-18a4-4e28-8aef-5bd4c38841a3" />
<img width="804" height="647" alt="74e3de6838bc3ddde203f4dd6461bca5" src="https://github.com/user-attachments/assets/c0fbfaf5-c676-4461-97f1-35a841f64e27" />

2026/8/20
大改了一波纹理,顺便把雪地和泥土的纹理从高度改成了流量,发现效果贼好,顺便把颜色从之前cpu算都移植到了frag里面算,当时我把worldUp不小心改成010,发现效果极佳!!!索性就把之前的法线权重改成dir这了,效果竟然和真的一样,雪地代码也大改了,为了加这个流量又重写了一波model,不过这个雪地太高的时候还是会出现有点问题,没那这滑下来的感觉,明天修一波,地形马上做完了,接下来做湖泊和海洋吧
<img width="1370" height="813" alt="2e452b485b3d38e124d44b131728a496" src="https://github.com/user-attachments/assets/83ff3b64-2550-4994-8046-98b2bb54d957" />
<img width="1370" height="813" alt="142d3bafe3ce540939af9dd880dd86de" src="https://github.com/user-attachments/assets/57928593-696d-4564-9881-f21795718c63" />
<img width="1370" height="813" alt="6284a9ab74ee20bd1546151de97f8fb6" src="https://github.com/user-attachments/assets/269dc9f2-451a-4ba0-b302-f0cdae521a9a" />
