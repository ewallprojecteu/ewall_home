package eu.ewall.platform.local.reasoners.properties;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

public class PropertyUtils {
	
	public static Properties getMessages(String code) {
		
		Properties msg = new Properties();
//		InputStream stream = null;
		
		try {
//			stream = new FileInputStream("messages_"+code+".properties");
			msg.load(PropertyUtils.class.getResourceAsStream("/messages_"+code+".properties"));
			
		} catch(IOException e) {
			e.printStackTrace();
		} finally {
//			if (stream != null) {
//				try {
//					stream.close();
//				} catch (IOException e) {
//					e.printStackTrace();
//				}
//			}
		}
		
		return msg;
	}

	public static Properties getThresholds() {
		Properties threshold = new Properties();
		
		try {
			threshold.load(PropertyUtils.class.getResourceAsStream("/threshold.properties"));
		} catch (IOException e) {
			e.printStackTrace();
		}
			
		
		return threshold;
	}

	public static Map<String, Properties> loadAllLanguages() {
		Map<String, Properties> languagesMap = new HashMap<String, Properties>();
		String[] codes = {"en","da","de","it","nl"};
		
		for(int it=0;it<codes.length;it++) {
			languagesMap.put(codes[it], getMessages(codes[it]));
		}
		
		
		return languagesMap;
	}
}
