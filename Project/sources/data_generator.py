import random
import datetime
import argparse
import sys

class DataGenerator:
    def __init__(self):
        # 1. 基础素材：常用汉字（用于合成词汇）
        self.common_chars = (
            "的一是了我不人在他有这个上们来到时大地为子中你说生国年着就那"
            "和要她出也得里后自以会家可下而过天去能对小多然于心学之都好看"
            "起发当没成只如事把还用第样道想作种开美总从无情己面最女但现前"
            "些所同日手又行意动方期它头经长儿回位分爱老因很给名法间斯知世"
            "什两次使身者被高已亲其进此话常与活正感"
        )
        
        # 2. 生成词汇表 (2000个常用词 + 500个停用词)
        print("正在初始化词汇表...", end="", flush=True)
        self.vocab = self._generate_vocab(2000)
        self.stop_words = self._generate_vocab(500)
        
        # 3. 计算 Zipf 分布权重 (核心：模拟热词效应)
        # 排名越靠前，权重越高 (权重 = 1 / rank)
        self.vocab_weights = [1.0 / (i + 1) for i in range(len(self.vocab))]
        print("完成")

    def _generate_vocab(self, size):
        """生成指定数量的随机中文词（模拟真实词汇）"""
        words = set()
        while len(words) < size:
            # 随机组合1-3个汉字形成一个词
            length = random.choice([1, 2, 2, 2, 3]) 
            word = "".join(random.choices(self.common_chars, k=length))
            words.add(word)
        return list(words)

    def generate_sentence(self):
        """生成符合长度要求的句子"""
        # 目标长度：10-50个字符
        target_len = random.randint(10, 50)
        current_len = 0
        sentence_parts = []

        while current_len < target_len:
            # 80% 概率选择常用词（受 Zipf 分布控制），20% 概率插入停用词
            if random.random() < 0.8:
                # 使用 weights 参数实现长尾分布：前面的词出现概率极大
                word = random.choices(self.vocab, weights=self.vocab_weights, k=1)[0]
            else:
                word = random.choice(self.stop_words)
            
            sentence_parts.append(word)
            current_len += len(word)
        
        # 截断或补齐以确保格式整洁（这里简单拼接）
        return "".join(sentence_parts)[:target_len]

    def run(self, line_count, output_file):
        """执行生成任务"""
        print(f"开始生成 {line_count} 行数据到 {output_file} ...")
        
        # 【修改点1】起始时间设为 0点0分0秒
        current_time = datetime.datetime.now().replace(hour=0, minute=0, second=0, microsecond=0)
        
        with open(output_file, "w", encoding="utf-8") as f:
            for i in range(line_count):
                # 模拟时间流逝：每生成 50-200 行，时间过 1 秒
                if i % random.randint(50, 200) == 0:
                    current_time += datetime.timedelta(seconds=1)
                
                
                # hour: 不补零 (0, 1, ..., 10)
                # minute:02d: 强制补零 (00, 05, ..., 59)
                # second:02d: 强制补零
                time_str = f"[{current_time.hour}:{current_time.minute:02d}:{current_time.second:02d}]"
                
                text = self.generate_sentence()
                
                f.write(f"{time_str} {text}\n")
                
                # 【修改点3】每隔 500 行数据，插入一条 ACTION 指令
                # (i + 1) 确保从第 500 行开始插入，而不是第 0 行
                if (i + 1) % 500 == 0:
                    f.write("[ACTION] QUERY K=50\n")
                
                if (i + 1) % (line_count // 10 if line_count >= 10 else 1) == 0:
                    print(f"进度: {((i+1)/line_count)*100:.0f}%")

        print(f"生成完毕！文件已保存: {output_file}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="热词分析系统 - 测试数据生成器")
    parser.add_argument("-n", "--count", type=int, default=10000, help="生成数据的行数 (默认: 10000)")
    parser.add_argument("-o", "--output", type=str, default="input.txt", help="输出文件名 (默认: input.txt)")
    
    args = parser.parse_args()
    
    generator = DataGenerator()
    generator.run(args.count, args.output)