let btnSubmit = document.querySelector("#btnSubmit");


function onSubmit(){
    
    console.log("Sender called");
}


btnSubmit.addEventListener('click',()=>{ onSubmit(); });