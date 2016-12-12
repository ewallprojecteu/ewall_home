package eu.ewall.sensing.socket;

public class InitInfo {
	String configFile;
	String dataLogDir;
	String couchdburl;
	String dbName;
	
	public String getConfigFile() {
		return configFile;
	}
	public void setConfigFile(String configFile) {
		this.configFile = configFile;
	}
	public String getDataLogDir() {
		return dataLogDir;
	}
	public void setDataLogDir(String dataLogDir) {
		this.dataLogDir = dataLogDir;
	}
	public String getCouchdburl() {
		return couchdburl;
	}
	public void setCouchdburl(String couchdburl) {
		this.couchdburl = couchdburl;
	}
	public String getDbName() {
		return dbName;
	}
	public void setDbName(String dbName) {
		this.dbName = dbName;
	}
}
