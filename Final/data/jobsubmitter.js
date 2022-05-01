let btnSubmit = document.querySelector("#btnSubmit");



function onSubmit(){
    const job = {
        BigTimeInterval: jobForm.BigTimeInterval[jobForm.BigTimeInterval.selectedIndex].value
    }

    fetch('/post',{method:'POST', body:JSON.stringify(job)});
    
}


btnSubmit.addEventListener('click',()=>{ onSubmit(); });