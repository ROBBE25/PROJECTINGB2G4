var temperature = msg.payload.temperature;
var humidity = msg.payload.humidity;
var co2 = msg.payload.co2;
var lokaal = msg.payload.lokaal;
var pr = msg.payload.pr; 
var noise = msg.payload.noise; 

const dataObject = new Date();
const timestamp = dataObject.toISOString();

// Constructing the SQL query
msg.topic = "INSERT INTO `data` (`timestamp`, `lokaal`, `temperature`, `humidity`, `co2`, `pr`, `noise`) VALUES ('"
    + timestamp + "', '"
    + lokaal + "', "
    + temperature + ", "
    + humidity + ", "
    + co2 + ", "
    + pr + ", "
    + noise  
    + ");";

return msg;