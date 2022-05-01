let content = document.querySelector('#content-area')
let btnCSV = document.querySelector('#CSV');
let btnChart = document.querySelector('#Chart');
let btnJob = document.querySelector('#AddJob');

// socket = new WebSocket()


document.addEventListener('readystatechange', event => { 
    // When window loaded ( external resources are loaded too- `css`,`src`, etc...) 
    if (event.target.readyState === "complete") {
        
    }
});


function showTable(text){
    var rows = text.split(/\r?\n/);
    var table = "<table><tr> <th>Date</th> <th>Time</th> <th>Value</th> </tr>";
    for (var i=0; i< rows.length; i++) {
        var cols = rows[i].split(",");
        table = table + "<tr>";
        for (var j=0; j< cols.length; j++){
            table = table + "<td>"+ cols[j] + "</td>"; 
        }
        table = table + "</tr>";
    }
    table = table + "</table>";
    content.innerHTML = table;
}

function readTextFile(file)
{
    var rawFile = new XMLHttpRequest();
    rawFile.open("GET", file, false);
    rawFile.onreadystatechange = function ()
    {
        if(rawFile.readyState === 4)
        {
            if(rawFile.status === 200 || rawFile.status == 0)
            {
                var allText = rawFile.responseText;
                showTable(allText);
            }
        }
    }
    rawFile.send(null);
}

function showJobMaker()
{
    content.innerHTML = '<iframe src="./jobmaker.html"></iframe>';
}

function showChart(){
    //TODO: edit this
}


btnCSV.addEventListener('click',()=>{ readTextFile("data.csv"); });

btnChart.addEventListener('click',()=>{ showChart(); });

btnJob.addEventListener('click',()=>{showJobMaker(); });