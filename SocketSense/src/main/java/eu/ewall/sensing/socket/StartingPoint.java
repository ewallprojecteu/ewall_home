package eu.ewall.sensing.socket;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collections;
import java.util.List;
import java.util.TimeZone;
import org.springframework.web.client.RestTemplate;
import com.fasterxml.jackson.databind.ObjectMapper;

public class StartingPoint {

	public static void main(String[] args) {
		Calendar date = Calendar.getInstance();

		// Read init.json
		ObjectMapper mapper = new ObjectMapper();
		InitInfo init = null;
		
		String path = StartingPoint.class.getProtectionDomain().getCodeSource().getLocation().getPath();
		try {
			path = URLDecoder.decode(path, "UTF-8");
		} catch (UnsupportedEncodingException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}
		path = path.substring(0, path.lastIndexOf("classes") + 1);
		path = path.substring(0, path.lastIndexOf("/") + 1) + "init.json";
		System.out.println("Reading initialisation info from: " + path);
		try {
			File file = new File(path);
			init = mapper.readValue(file, InitInfo.class);
		} catch (Exception e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		String configFile = init.getConfigFile();
		String dataLogDir = init.getDataLogDir();
		String couchdburl = init.getCouchdburl();
		String dbName = init.getDbName();

		SocketSensing socketSense = new SocketSensing(couchdburl, dbName, true);
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS'Z'");
		SimpleDateFormat sdfDate = new SimpleDateFormat("yyyy-MM-dd");
		sdf.setTimeZone(TimeZone.getTimeZone("GMT"));
		//sdfDate.setTimeZone(TimeZone.getTimeZone("GMT"));
		
		// Read Plugwise initialisation information...
		mapper = new ObjectMapper();
		CircleInfo info = null;
		try {
			File file = new File(configFile);
			info = mapper.readValue(file, CircleInfo.class);
		} catch (Exception e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		// Setup sockets
		List<File> circleLogs = new ArrayList<File>();
		for (Circle circle:info.getCircles()) {
			socketSense.addAppliance(circle.getLocation(), circle.getCategory(), circle.getName());
			circleLogs.add(new File(""));
		}
		// Find start time
		int year, month, day;
		String yearLogDir;
		RestTemplate restTemplate = new RestTemplate();
		String json = restTemplate.getForObject(couchdburl + "/" + dbName + "/_all_docs?descending=true&limit=1", String.class);
		try {
			String id = json.substring(json.indexOf("\"id\"")+6, json.indexOf("\"id\"")+30);
			json = restTemplate.getForObject(couchdburl + "/" + dbName + "/" +id, String.class);
			for (int i=0; i<socketSense.getSockets().size(); i++) {
				id = json.substring(json.indexOf("\"" + socketSense.getSockets().get(i).getName() + "\""));
				id = id.substring(id.indexOf("\"dailyEnergy\"")+14);
				socketSense.getSockets().get(i).setDailyEnergy(Double.parseDouble(id.substring(0, id.indexOf(","))));
			}
			date.setTime(sdf.parse(json.substring(json.indexOf("\"timestamp\"")+13, json.indexOf("\"timestamp\"")+37)));
			date.setTimeInMillis(date.getTimeInMillis() + 1000);
			year = date.get(Calendar.YEAR);
			month = date.get(Calendar.MONTH);
			day = date.get(Calendar.DAY_OF_MONTH);
		} catch (Exception e1) {
			// Something is wrong, most probably nothing in database, find the very first log entry
			File dir = new File(dataLogDir);
			String [] dirs = dir.list(new FilenameFilter() {
				@Override
				public boolean accept(File current, String name) {
					return new File(current, name).isDirectory();
				}
			});
			List<String> dirList = Arrays.asList(dirs);
			Collections.sort(dirList);
			year = Integer.parseInt(dirList.get(0));
			month = 0;
			day = 1;
			date.set(year, month, day, 0, 0, 0);
		}
		
		int hourReported = -1;
		Boolean preliminaryFile = false;
		socketSense.setChanged(false);
		while (true) {
			// Find first file to read for all monitored sockets
			yearLogDir = dataLogDir + Integer.toString(year) +"/pwlog/";
			for (int i = 0; i<circleLogs.size(); i++) {
				if (!preliminaryFile) {
					System.out.println(sdfDate.format(date.getTime()) + ": Looking for log file...");
					circleLogs.set(i, new File(yearLogDir + "pw-" + sdfDate.format(date.getTime()) + "-" + info.getCircles().get(i).getMac() + ".log"));
					if (!circleLogs.get(i).exists()) {
						System.out.println(sdfDate.format(date.getTime()) + ": No log file for that date, looking for preliminary one...");
						circleLogs.set(i, new File(yearLogDir + "pw-" + info.getCircles().get(i).getMac() + ".log"));
						preliminaryFile = true;
						if (!circleLogs.get(i).exists()) {
							// No preliminary file, increase date until something is found...
							// Check if there is a file with next date
							preliminaryFile = false;
							System.out.println("No preliminary log file, increasing dates...");
							Calendar temp = Calendar.getInstance();
							day++;
							temp.set(year, month, day, 0, 0, 0);
							System.out.println(sdfDate.format(temp.getTime()) + ": Looking for log file...");
							File file = new File(yearLogDir + "pw-" + sdfDate.format(temp.getTime()) + "-" + info.getCircles().get(i).getMac() + ".log");
							while (!file.exists()) {
								day++;
								temp.set(year, month, day, 0, 0, 0);
								System.out.println(sdfDate.format(temp.getTime()) + ": Looking for log file...");
								file = new File(yearLogDir + "pw-" + sdfDate.format(temp.getTime()) + "-" + info.getCircles().get(i).getMac() + ".log");
							}
							date = temp;
							System.out.println("Starting with log file " + yearLogDir + "pw-" + sdfDate.format(date.getTime()) + "-" + info.getCircles().get(i).getMac() + ".log");
							circleLogs.set(i, file);
						}
					}
				} else {
					circleLogs.set(i, new File(yearLogDir + "pw-" + info.getCircles().get(i).getMac() + ".log"));
				}
				
				while (!circleLogs.get(i).exists()) {
					System.out.println(sdfDate.format(date.getTime()) + ": Waiting for log file to be written...");
					try {
						Thread.sleep(10000);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
			// Read log files
			List<Socket> sockets = socketSense.getSockets();
			Long millis = (long) 0;
			int foundInFile = 0;
			for (int i = 0; i<circleLogs.size(); i++) {
				List<String> lines = null;
				try {
					lines = Files.readAllLines(Paths.get(circleLogs.get(i).toString()), Charset.forName("UTF-8"));
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				for(String line:lines){
					List<String> tockens = Arrays.asList(line.split(","));
					if (date.getTimeInMillis() / 1000 > Long.parseLong(tockens.get(0)))
						continue;
					millis = Long.parseLong(tockens.get(0)) * 1000;
					Calendar temp = Calendar.getInstance();
					temp.setTimeInMillis(millis);
					if (day != temp.get(Calendar.DAY_OF_MONTH)) {
						sockets.get(i).setDailyEnergy(0);
					}
					double power = Double.parseDouble(tockens.get(1));
					System.out.println("Socket " + sockets.get(i).getName() + " at " + sdf.format(temp.getTime()));
					System.out.println("\tHour reported " + hourReported + ", now " + temp.get(Calendar.HOUR_OF_DAY));
					System.out.println("\tPower reported " + sockets.get(i).getPowerNow() + ", now " + power);
					if (Math.abs(sockets.get(i).getPowerNow() - power) > power / 100 || hourReported != temp.get(Calendar.HOUR_OF_DAY)) {
						sockets.get(i).setPowerNow(power);
						socketSense.setChanged(true);
						System.out.println("\tSo, reporting!");
					}
					sockets.get(i).setDailyEnergy(sockets.get(i).getDailyEnergy() + Double.parseDouble(tockens.get(2)));
					foundInFile ++;
					break;
				}
			}
			if (foundInFile == circleLogs.size()) {
				// File still has records to process
				date.setTimeInMillis(millis);
				socketSense.setSockets(sockets);
				if (socketSense.isChanged())
					hourReported = date.get(Calendar.HOUR_OF_DAY);
				System.out.println("** Ready to write to CouchDB (if needed)");
				socketSense.put2db(sdf.format(date.getTime()));
				date.setTimeInMillis(millis + 1000);
				year = date.get(Calendar.YEAR);
				month = date.get(Calendar.MONTH);
				day = date.get(Calendar.DAY_OF_MONTH);
			} else {
				// No more records
				if (preliminaryFile) {
					// If in preliminary file, force processing of next date file
					preliminaryFile = false;
				} else {
					// Check if there is a file with next date
					Calendar temp = Calendar.getInstance();
					temp.set(year, month, day + 1, 0, 0, 0);
					File file = new File(yearLogDir + "pw-" + sdfDate.format(temp.getTime()) + "-" + info.getCircles().get(0).getMac() + ".log");
					if (file.exists()) {
						date = temp;
					} else {
						// If in date file, wait for next line to be written
						System.out.println(sdfDate.format(date.getTime()) + ": Waiting for log file to be written...");
						try {
							Thread.sleep(10000);
						} catch (InterruptedException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					}
				}
			}
		}
	}
}
