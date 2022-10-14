/* ***************** Captive Portal *********************/
IPAddress cpIP(192, 168, 10, 1);  // Captive Portal IP-Adresse - outside of the router's own DHCP range

String ProgramName = WIFI_HOSTNAME;

// *****************************************************************************************************************************************************

String ipToString(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

// *****************************************************************************************************************************************************

void CaptivePortal(){
// Start Captive Portal (Access Point)
WiFi.mode(WIFI_AP);
WiFi.softAPConfig(cpIP, cpIP, IPAddress(255, 255, 255, 0));   //Captive Portal IP-Adress
WiFi.softAP(ProgramName, "");
// Webserver - Ausgabe wenn Link nicht gefunden wurde
server.onNotFound(WebSiteNotFound);
server.on("/", handlePortal);
server.begin();  
}

// *****************************************************************************************************************************************************

void handlePortal() {
String HTMLString; 
String ssid; // for Sort SSID's
int loops = 0; // for Sort SSID's
if (ProgramName == "") {ProgramName = "Hostname";} 
// Wenn gespeichert Button betätigt wurde 
if (server.method() == HTTP_POST) {
    // Einstellungen speichern
    WIFI_SSID = server.arg("ssid"); // Wifi SSID
    WIFI_PASS = server.arg("password"); // Wifi SSID Passwort
    OPEN_WEATHER_MAP_API_KEY = server.arg("apikey"); // OpenWeatherMap - API-Key
    OPEN_WEATHER_MAP_LOCATION_ID = server.arg("cityid"); // OpenWeatherMap - City ID
    UWD = server.arg("uwd"); // OpenWeatherMap - Update-Interval in Minuten

    if (WIFI_SSID != "" && WIFI_PASS != "" && OPEN_WEATHER_MAP_API_KEY != "" && OPEN_WEATHER_MAP_LOCATION_ID != "") { 
    Serial.println("SSID: "+WIFI_SSID); // Wifi SSID
    Serial.println("Passwort: "+WIFI_PASS); // Wifi SSID Passwort

    if (isFSMounted == true){
    savePropertiesToLittlefs(); // Eingaben im FileSystem speichern
    Serial.println("Zugangsdaten wurden gespeichert");
    HTMLString = "<!doctype html><html lang='de'>";
    HTMLString += "<head><meta charset='utf-8'>";
    HTMLString += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    HTMLString += "<title>WiFi-Manager</title>";
    HTMLString += "<style>";
    HTMLString += "*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:";
    HTMLString += "'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;";
    HTMLString += "font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:";
    HTMLString += "block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid";
    HTMLString += "transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;";
    HTMLString += "line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}";
    HTMLString += "h5 { font-size: 24px; text-align:center; margin-top: 0px; margin-bottom: 10px;}"; 
    HTMLString += "h6 { font-size: 18px; text-align:center; margin-top: 0px; margin-bottom: 15px;}"; 
    HTMLString += "</style>";
    HTMLString += "</head>";
    HTMLString += "<body><main class='form-signin'>";
    HTMLString += "<h1>WiFi-Manager</h1>";
    HTMLString += "<h5>("+ProgramName+")</h5>";
    HTMLString += "<br/>";
    HTMLString += "<h6>Die Einstellungen wurden gespeichert<br />Die Wetterstation wird neu gestartet</h6>";
    HTMLString += "</main>";
    HTMLString += "</body>";
    HTMLString += "</html>";
    server.send(200, "text/html", HTMLString); // Captive Portal 
    // Reset auslösen
    Serial.println("Wetterstation wird neu gestartet");
    MyWaitLoop(3000);
    ESP.restart();  
    }
     else
     { // Zugangsdaten wurden nicht gespeichert
     Serial.println("Zugangsdaten wurden nicht gespeichert !");
     HTMLString = "<!doctype html><html lang='de'>";
    HTMLString += "<head><meta charset='utf-8'>";
    HTMLString += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    HTMLString += "<title>WiFi-Manager</title>";
    HTMLString += "<style>";
    HTMLString += "*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:";
    HTMLString += "'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;";
    HTMLString += "font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:";
    HTMLString += "block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid";
    HTMLString += "transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;";
    HTMLString += "line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}";    
    HTMLString += "h5 { font-size: 24px; text-align:center; margin-top: 0px; margin-bottom: 10px;}"; 
    HTMLString += "h6 { font-size: 18px; text-align:center; margin-top: 0px; margin-bottom: 15px;}"; 
    HTMLString += "</style>";
    HTMLString += "</head>";
    HTMLString += "<body><main class='form-signin'>";
    HTMLString += "<h1>WiFi-Manager</h1>";
    HTMLString += "<h5>("+ProgramName+")</h5>";
    HTMLString += "<br/>";
    HTMLString += "<h6>Fehler beim Speichern der Einstellungen<br />Die Wetterstation wird neu gestartet</h6>";
    HTMLString += "</main>";
    HTMLString += "</body>";
    HTMLString += "</html>";
    server.send(200, "text/html", HTMLString); // Captive Portal 
     // Reset auslösen 
     Serial.println("Wetterstation wird neu gestartet");
     MyWaitLoop(3000);
     ESP.restart();  
    } 
    } else
       {
    // unvollständige Einstellungen    
    HTMLString =  "<!doctype html><html lang='de'>";
    HTMLString += "<head><meta charset='utf-8'>";
    HTMLString += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    HTMLString += "<title>WiFi-Manager</title>";
    HTMLString += "<style>";
    HTMLString += "*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:";
    HTMLString += "'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;";
    HTMLString += "font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:";
    HTMLString += "block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid";
    HTMLString += "transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;";
    HTMLString += "line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}";
    HTMLString += "h5 { font-size: 24px; text-align:center; margin-top: 0px; margin-bottom: 10px;}"; 
    HTMLString += "h6 { font-size: 18px; text-align:center; margin-top: 0px; margin-bottom: 15px;}"; 
    HTMLString += "</style>";
    HTMLString += "</head>";
    HTMLString += "<body><main class='form-signin'>";
    HTMLString += "<h1>WiFi-Manager</h1>";
    HTMLString += "<h5>("+ProgramName+")</h5>";
    HTMLString += "<br/>";
    HTMLString += "<h6>Die Einstellungen sind unvollständig !<br />Einstellungen wurden nicht gespeichert</h6>";
    HTMLString += "<br/>";
    HTMLString += "<a href =\"/\"><button class=\"button\">zurück zur Startseite</button></a>";
    HTMLString += "</main>";
    HTMLString += "</body>";
    HTMLString += "</html>";}
    server.send(200, "text/html", HTMLString);  // Speichern   
  } else {
    // Captive Portal Einstellungen
    int n = WiFi.scanNetworks(false, false); //WiFi.scanNetworks(async, show_hidden)
    HTMLString =  "<!doctype html><html lang='de'>";
    HTMLString += "<head><meta charset='utf-8'>";
    HTMLString += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    HTMLString += "<title>WiFi-Manager</title>";
    HTMLString += "<style>";
    HTMLString += "*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:";
    HTMLString += "'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;";
    HTMLString += "font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:";
    HTMLString += "block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid";
    HTMLString += "transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;";
    HTMLString += "line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}";
    HTMLString += "h5 { font-size: 24px; text-align:center; margin-top: 0px; margin-bottom: 10px;}"; 
    HTMLString += "h6 { font-size: 18px; margin-left: 110px; margin-top: 15px; margin-bottom: 5px; color: #0245b0;}";
    HTMLString += "h7 { font-size: 20px; font-weight: bold; margin-left: 110px; margin-top: 0px; margin-bottom: 5px; color: #06942c;}";
    HTMLString += "</style>";
    HTMLString += "</head>";
    HTMLString += "<body><main class='form-signin'>";
    HTMLString += "<form action='/' method='post'>";
    HTMLString += "<h1>WiFi-Manager</h1>";
    HTMLString += "<h5>("+ProgramName+")</h5>";
    HTMLString += "<br/>";
    if (n > 0) {
     // WLAN's sort by RSSI
    int indices[n];
    int skip[n];
    int o = n;
    for (int i = 0; i < n; i++) {
    indices[i] = i;}
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
          if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            loops++;
            //int temp = indices[j];
            //indices[j] = indices[i];
            //indices[i] = temp;
            std::swap(indices[i], indices[j]);
            std::swap(skip[i], skip[j]);}}}
      for (int i = 0; i < n; i++) {
        if(indices[i] == -1){
          --o;
          continue;}
        ssid = WiFi.SSID(indices[i]);
        for (int j = i + 1; j < n; j++) {
          loops++;
          if (ssid == WiFi.SSID(indices[j])) {
            indices[j] = -1;}}}
    for (int i = 0; i < n; ++i){
    // Print SSID 
    if (i == 0) {
    HTMLString += "<h7 onclick='SetSSID"+(String)i+"()' id='fssid"+(String)i+"'>" + WiFi.SSID(indices[i]) + "</h7>";} else {
    HTMLString += "<h6 onclick='SetSSID"+(String)i+"()' id='fssid"+(String)i+"'>" + WiFi.SSID(indices[i]) + "</h6>";}
    HTMLString += "<script>";
    HTMLString += "function SetSSID"+(String)i+"() {document.getElementById('ssid').value = document.getElementById('fssid"+(String)i+"').innerHTML;}";
    HTMLString += "</script>";}
    } else {
    HTMLString += "<br/>";  
    HTMLString += "<br/>"; 
    HTMLString += "<h6>kein WLAN gefunden !</h6>";
    HTMLString += "<br/>";}
    HTMLString += "<br/>";
    HTMLString += "<div class='form-floating'><label>SSID</label><input type='text' class='form-control' name='ssid' id='ssid' value='' id='CPSSID'></div>";
    HTMLString += "<div class='form-floating'><br/><label>Password</label><input type='password' class='form-control' name='password' id='password' value=''></div>";
    HTMLString += "<br/>"; 
    // ******************************************* Parameter ************************************************************************************************************************************
    HTMLString += "<div class='form-floating'><br/><label>OpenWeatherMap - API-Key</label><input type='text' class='form-control' name='apikey' id='apikey' value='"+OPEN_WEATHER_MAP_API_KEY+"'></div>";
    HTMLString += "<div class='form-floating'><br/><label>OpenWeatherMap - City-ID</label><input type='text' class='form-control' name='cityid' id='cityid' value='"+OPEN_WEATHER_MAP_LOCATION_ID+"'></div>";
    HTMLString += "<div class='form-floating'><br/><label>OpenWeatherMap - Aktualisierung in Minuten</label><input type='text' class='form-control' name='uwd' id='uwd' value='"+UWD+"'></div>";
    // ******************************************* Parameter ************************************************************************************************************************************
    HTMLString += "<br/><br/>";
    HTMLString += "<button type='submit'>Speichern</button>";
    HTMLString += "</form>";
    HTMLString += "<br/><br/>";
    HTMLString += "<form action='/' method='get'>";
    HTMLString += "<button type='submit'>Aktualisieren</button>";
    HTMLString += "</form>";
    HTMLString += "</main>";
    HTMLString += "</body>";
    HTMLString += "</html>";
    server.send(200, "text/html", HTMLString); // Captive Portal 
  }
}

// *****************************************************************************************************************************************************

void WebSiteNotFound() {
String HTMLString;   
HTMLString =  "<!doctype html><html lang='de'>";
HTMLString += "<head><meta charset='utf-8'>";
HTMLString += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
HTMLString += "<title>WiFi-Manager</title>";
HTMLString += "<style>";
HTMLString += "*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:";
HTMLString += "'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;";
HTMLString += "font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:";
HTMLString += "block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid";
HTMLString += "transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;";
HTMLString += "line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}";
HTMLString += "h5 { font-size: 24px; text-align:center; margin-top: 0px; margin-bottom: 10px;}"; 
HTMLString += "h6 { font-size: 20px; text-align:center; margin-top: 0px; margin-bottom: 10px;}"; 
HTMLString += "</style>";
HTMLString += "</head>";
HTMLString += "<body><main class='form-signin'>";
HTMLString += "<h1>WiFi-Manager</h1>";
HTMLString += "<h5>("+ProgramName+")</h5>";
HTMLString += "<br/>";
HTMLString += "<h6>Die Webseite wurde nicht gefunden !</h6>";
HTMLString += "<br/><br/>";
HTMLString += "<a href =\"/\"><button class=\"button\">zurück zur Startseite</button></a>";
HTMLString += "</main>";
HTMLString += "</body>";
HTMLString += "</html>";    
server.send(200, "text/html", HTMLString);   
} 

// *****************************************************************************************************************************************************
// *****************************************************************************************************************************************************
