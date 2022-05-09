let btnSubmit = document.querySelector("#btnSubmit");



function onSubmit(){
    const job = {
        BigTimeInterval: jobForm.BigTimeInterval[jobForm.BigTimeInterval.selectedIndex].value,
        BigCount: jobForm.BigCount[jobForm.BigCount.selectedIndex].value,
        SmallTimeInterval: jobForm.SmallTimeInterval[jobForm.SmallTimeInterval.selectedIndex].value,
        SmallCount: jobForm.SmallCount[jobForm.SmallCount.selectedIndex].value
    }
    //Make POST request
    fetch('/post',{method:'POST', body:JSON.stringify(job)});
}


btnSubmit.addEventListener('click',()=>{ onSubmit(); });