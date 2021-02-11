import java.util.Scanner;
import java.io.File;
import java.io.FileWriter;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Map;
import java.util.HashMap;
import java.util.Arrays;

public class Population  {
	public static void main(String[] args) {
		File source = new File(args[0]);
		File macro = new File(args[1]);
		File state = new File(args[2]);
		File county = new File(args[3]);

		Map<String, Integer> macroPopulation = new HashMap<String, Integer>();
		Map<String, Integer> ufPopulation = new HashMap<String, Integer>();
		Map<String, Integer> munPopulation = new HashMap<String, Integer>();
		try {
			Scanner in = new Scanner(source);
			in.nextLine();
			while (in.hasNextLine()) {
				String[] sourceLine = in.nextLine().split(",");

				Integer prevValue1 = macroPopulation.get(sourceLine[1]);
				if (prevValue1==null) prevValue1 = 0;
				macroPopulation.put(sourceLine[1], prevValue1 + Integer.parseInt(sourceLine[5]));

				Integer prevValue2 = ufPopulation.get(sourceLine[3]);
				if (prevValue2==null) prevValue2 = 0;
				ufPopulation.put(sourceLine[3], prevValue2 + Integer.parseInt(sourceLine[5]));

				munPopulation.put(sourceLine[0], Integer.parseInt(sourceLine[5]));
			}
			in.close();
		}
		catch (FileNotFoundException error) {
			error.printStackTrace();
		}

		try {
			Scanner in = new Scanner(macro);
			FileWriter fw = new FileWriter("new_" + args[1]);
			while (in.hasNextLine()) {
				String macroLine = in.nextLine();
				fw.write(macroLine + "\n");
				if (macroLine.contains("\"isocode\"")) {
					String[] macroLineSplit = macroLine.split("\"");
					String macroName = macroLineSplit[macroLineSplit.length - 2];
					fw.write("\"population\": " + macroPopulation.get(macroName) + ",\n");
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

		try {
			Scanner in = new Scanner(state);
			FileWriter fw = new FileWriter("new_" + args[2]);
			while (in.hasNextLine()) {
				String stateLine = in.nextLine();
				fw.write(stateLine + "\n");
				if (stateLine.contains("\"UF_05\"")) {
					String[] stateLineSplit = stateLine.split("\"");
					String ufName = stateLineSplit[stateLineSplit.length - 2];
					fw.write("\"population\": " + ufPopulation.get(ufName) + ",\n");
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

		try {
			Scanner in = new Scanner(county);
			FileWriter fw = new FileWriter("new_" + args[3]);
			while (in.hasNextLine()) {
				String countyLine = in.nextLine();
				fw.write(countyLine + "\n");
				if (countyLine.contains("\"GEOCODIGO\"")) {
					String munName = countyLine.substring(countyLine.indexOf(":") + 1);
					munName = munName.substring(0, munName.indexOf(","));
					munName = munName.trim();
					fw.write("\"population\": " + munPopulation.get(munName) + ",\n");
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
