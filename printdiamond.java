package Basic3;

public class printdiamond {

	public static void main(String[] args) {
		//1 輸出上半部分
		int starLine = 4;
		for (int i =1; i<=starLine;i++) {
			//1.1輸出空白部分
			for(int j=1; j<= starLine -i ; j++) {
				System.out.print(" ");
			}
			//1.2輸出*
			for(int k=1; k<=2*i-1;k++) {
				System.out.print("*");
			}
			System.out.println();
		}
		//2.輸出下半部分
		for(int i=1; i<=starLine-1; i++) {
			//2.1輸出空格
			for(int j =1; j<=i; j++) {
				System.out.print(" ");
			}
			//2.2輸出*
			for(int k=1; k<=(-2 * i+2*starLine-1);k++) {
				System.out.print("*");
			}
			System.out.println();
		}
	}

}
