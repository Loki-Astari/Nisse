
function pushBut(but) {
    alert("Button Pushed: " + but);
}

function updatePersonField(id, value) {
    field =  document.getElementById(id);
    field.value = value;
}

function loadPerson(id) {
    const userAction = async () => {
        console.log("Get: " + id);
        request = [window.location.protocol + '//' + window.location.host, '/person/Id-', id].join('');
        const response = await fetch(request);
        const person = await response.json();
        console.log("%j", person);

        updatePersonField('personId', person._id);
        updatePersonField('personNameF', person.name.first);
        updatePersonField('personNameL', person.name.last);
        updatePersonField('personAge', person.age);
        updatePersonField('personStreet1', person.contactInfo.address.street1);
        updatePersonField('personStreet2', person.contactInfo.address.street2);
        updatePersonField('personCity', person.contactInfo.address.city);
        updatePersonField('personState', person.contactInfo.address.state);
        updatePersonField('personZip', person.contactInfo.address.zip);
        updatePersonField('personTelType', person.contactInfo.telephone.type);
        updatePersonField('personTelNum', person.contactInfo.telephone.number);
    }
    userAction();
}

function getPerson() {
    console.log("getPerson()");
    id = document.getElementById('personId').value;
    loadPerson(id);
}

function updatePerson() {
    request = [window.location.protocol + '//' + window.location.host, '/person/Id-', document.getElementById('personId').value].join('');
    update('PUT', request);
}
function addPerson() {
    request = [window.location.protocol + '//' + window.location.host, '/person/'].join('');
    update('POST', request);
}

function update(method, request) {
    person = {
        name: {
            first: document.getElementById('personNameF').value,
            last: document.getElementById('personNameL').value
        },
        age: parseInt(document.getElementById('personAge').value),
        contactInfo: {
            address: {
                street1: document.getElementById('personStreet1').value,
                street2: document.getElementById('personStreet2').value,
                city: document.getElementById('personCity').value,
                state: document.getElementById('personState').value,
                zip: document.getElementById('personZip').value
            },
            telephone: {
                type: document.getElementById('personTelType').value,
                number: document.getElementById('personTelNum').value
            }
        }
    };
    console.log("request: " + request);
    const userAction = async () => {
        console.log("String: >" + JSON.stringify(person) + "<");
        const response = await fetch(request, {
                                        method: method,
                                        body:JSON.stringify(person)
                                     });
        const result = await response.json();
        console.log("response: %j", result);
        newId = result[0];
        document.getElementById('personId').value = newId;
        console.log("New Id: " + newId);
    }
    userAction();
}


function findByZip() {
    console.log("findByZip()");
    zip = document.getElementById('findByZip').value;
    request = [window.location.protocol + '//' + window.location.host, '/person/findByZip/', zip].join('');
    findAndUpdate(request);
}

function findByTel() {
    console.log("findByTel()");
    tel = document.getElementById('findByTelephone').value;
    request = [window.location.protocol + '//' + window.location.host, '/person/findByTel/', tel].join('');
    findAndUpdate(request);
}

function findByName() {
    console.log("findByNAme()");
    first = document.getElementById('findByNameFirst').value;
    last  = document.getElementById('findByNameLast').value;
    request = [window.location.protocol + '//' + window.location.host, '/person/findByName/', first, '/', last].join('');
    findAndUpdate(request);
}

function findAndUpdate(request)
{
    console.log("request: " + request);
    const userAction = async () => {
        const response = await fetch(request);
        const result = await response.json();
        console.log("response: " + result);

        updateFindList(result);
    }
    userAction();
}

function updateFindList(result)
{
    list = document.getElementById('foundList');
    list.replaceChildren();
    result.forEach(function(item, index) {
        console.log(item._id);
        divText = document.createTextNode(item._id);
        divElement = document.createElement('div');
        divElement.appendChild(divText);
        divElement.setAttribute("onclick","loadPerson('" + item._id + "')");
        list.appendChild(divElement);
    });
}

