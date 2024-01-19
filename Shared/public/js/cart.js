// This file contains the cart object and relative tasks
//(()=>{
    class Cart {
        constructor() {
            // this.x = 1
            // this.y = 2
        }


        getProfileLink(profileID) {
            let domain = window.location.origin;

            return `${origin}/users/${profileID}`;
        }
    }

function getCart(userID)
{
    const Http = new XMLHttpRequest();
    const url = 'http://localhost:8080/cart/0'
    Http.open("GET", url);
    Http.send();

    Http.onreadystatechange=(e)=>{
        console.log(Http.responseText);
    }
}

//});