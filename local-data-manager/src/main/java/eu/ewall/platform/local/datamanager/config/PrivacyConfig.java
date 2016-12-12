package eu.ewall.platform.local.datamanager.config;

import java.io.IOException;
import java.io.InputStream;

import org.apache.commons.configuration.ConfigurationException;
import org.apache.commons.configuration.PropertiesConfiguration;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class PrivacyConfig implements LocalDataManagerConfig {
	
	/** The config file. */
	private String privacyConfigFile = "/privacy.properties";
	
	/** The prop. */
	PropertiesConfiguration prop = new PropertiesConfiguration();
	
	/** The logger. */
	Logger logger = LoggerFactory.getLogger(PrivacyConfig.class);
	
	public PrivacyConfig(){
		loadProperties();
	}
	
	@Override
	public PropertiesConfiguration getProperties() {
		// TODO Auto-generated method stub
		return prop;
	}

	@Override
	public PropertiesConfiguration loadProperties() {
		prop = new PropertiesConfiguration();
		try {
			InputStream in = this.getClass().getResourceAsStream(privacyConfigFile);
			prop.load(in);
			in.close();
			logger.info("Successfully loaded privacy properties from file: {}", privacyConfigFile);			
		} catch (ConfigurationException e) {
			logger.error("Error in loading privacy properties from file: {}", privacyConfigFile, e);
		} catch (IOException e) {
			logger.error("Error in loading privacy properties from file: {}", privacyConfigFile, e);
		}		
		return prop;
	}

	@Override
	public boolean setProperties(PropertiesConfiguration prop) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public String getConfigFileName() {
		// TODO Auto-generated method stub
		return privacyConfigFile;
	}

}
