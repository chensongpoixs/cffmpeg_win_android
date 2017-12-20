http://ffmpeg.org/releases/


chmod 777 -R ffmpeg-2.6.9


--disable 



获取java签名   

javap -s 包名类名


pcm opense

1. 初始化
2. 

DTS和PTS
上面提到，视频和音频的同步过程是一个你等我赶的过程，
快了则等待，慢了就加快速度。这就需要一个量来判断
（和选择基准比较），到底是播放的快了还是慢了，
或者正以同步的速度播放。在视音频流中的包中都含有DTS和PTS，
就是这样的量（准确来说是PTS）。
DTS，Decoding Time Stamp， 解码时间戳，
告诉解码器packet的解码顺序；PTS，Presentation Time Stamp， 显示时间戳，
指示从packet中解码出来的数据的显示顺序。 
视音频都是顺序播放的，其解码的顺序不应该就是其播放的顺序么，
为啥还要有DTS和PTS之分呢。对于音频来说， DTS和PTS是相同的，
也就是其解码的顺序和解码的顺序是相同的，但对于视频来说情况就有些不同了。 
视频的编码要比音频复杂一些，特别的是预测编码是视频编码的基本工具，
这就会造成视频的DTS和PTS的不同。

这样视频编码后会有三种不同类型的帧：
I帧 关键帧，包含了一帧的完整数据，解码时只需要本帧的数据，不需要参考其他帧。
P帧 P是向前搜索，该帧的数据不完全的，解码时需要参考其前一帧的数据。
B帧 B是双向搜索，解码这种类型的帧是最复杂，不但需要参考其一帧的数据，还需要其后一帧的


注意，这里的pts是double型，因为将其乘以了time_base，代表了该帧在视频中的时间
位置（秒为单位）。有可能存在调用av_frame_get_best_effort_timestamp 得不到一个
正确的PTS，这样的情况放在函数synchronize 中处理。



###H264 分析
LALU

0x67   SPS   序列参数集profile, level, 宽, 高, 与颜色等信息
0x68   PPS   图像参数集, 通常情况下,PPS类似于SPS
0x65   I     关键帧,播放器需要从IDR开始播放, 发送IDR前需要发送SPS和PPS



