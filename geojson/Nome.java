import java.util.Scanner;
import java.io.File;
import java.io.FileWriter;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Map;
import java.util.HashMap;
import java.util.Arrays;

public class Nome  {
	public static void main(String[] args) {
		File source = new File(args[0]);
		File county = new File(args[1]);

		Map<String, String> munPopulation = new HashMap<String, String>();
		try {
			Scanner in = new Scanner(source);
			in.nextLine();
			while (in.hasNextLine()) {
				String[] sourceLine = in.nextLine().split(",");
				munPopulation.put(sourceLine[0]+sourceLine[1], sourceLine[2]);
			}
			in.close();
		}
		catch (FileNotFoundException error) {
			error.printStackTrace();
		}

		try {
			Scanner in = new Scanner(county);
			FileWriter fw = new FileWriter("new_" + args[1]);
			while (in.hasNextLine()) {
				String countyLine = in.nextLine();
				fw.write(countyLine + "\n");
				if (countyLine.contains("\"GEOCODIGO\"")) {
					String munName = countyLine.substring(countyLine.indexOf(":") + 1);
					munName = munName.substring(0, munName.indexOf(","));
					munName = munName.trim();
					fw.write("\"right_name\": \"" + munPopulation.get(munName) + "\",\n");
				}
			}
			in.close();
			fw.close();
		}
		catch (FileNotFoundException error) {
			error.printStackTrace();
		}
		catch (IOException error) {
			error.printStackTrace();
		}
	}
}
