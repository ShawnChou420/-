package Basic2;

public class Stock {
	//假設投資第一銀行股票10萬每年可以獲得6%獲利，
	//所獲得的6%獲利次年將變成本金繼續投資，請列出未來每20年每年本金和。

	public static void main(String[] args) {
		double rate = 0.06;//利率
		double capital = 100000;//本金
		double capitalInfo;
		for(int i = 1; i<=20; i++) {
			capitalInfo = capital * Math.pow((1.0+rate), i);
			//Math.pow(a, b)這方法用來回傳x的y次方值
			System.out.printf("第%2d年後本金和是%10.2f\n",i,capitalInfo);
		}

	}

}
