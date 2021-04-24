package Basic2;

import java.util.Scanner;
import java.util.Random;
public class lottery {
	public static void main(String[] args) {
		int number;										// 特別號
		int[] lottery = new int[50];
		Scanner scanner = new Scanner(System.in);
		Random rand = new Random();
		
		System.out.print("請輸入購買威力彩數量 : ");
		int num = scanner.nextInt();					// 讀取購買威力彩卷數量
		
		for ( int i = 1; i <= num; i++) {				// 處理主號		
			System.out.printf("%d : \t", i);			// 輸出第幾組資料
			for ( int n = 1; n <= 49; n++)				// 處理lottery[n]=n, n = 1-49
				lottery[n] = n;
			int counter = 1;							// 各組數字編號
			while ( counter <= 6 ) {					// 一組有6個數字
				int lotteryNum = rand.nextInt(49) + 1; 	// 產生主號碼
				if (lottery[lotteryNum] == 0)			// 如果是0表示此數字已經產生
					continue;							// 返回while迴圈
				else {
					System.out.printf("%d  \t", lotteryNum);	// 產生新的主號數字
					lottery[lotteryNum] = 0;			// 將此陣列索引設為0
					counter++;							// 將主號數字編號加1
				}
			}
			number = rand.nextInt(8) + 1;
			System.out.printf("   特別號 : %d%n", number);
		}
	}
}

