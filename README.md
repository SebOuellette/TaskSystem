# Shopping Cart Module

This is the shopping cart module for CSCN73030 Lab 2. 

## Documentation

### Legend
`<hello>` -- Text surrounded by <> is a variable. Do not include <> when using the API.<br>
`?foobar?` -- Text surrounded by ?? is content that is to be decided later in development. 


### Open User's Cart
### Request
`GET` `/<profile_id>`

### Link to Profile Module
#### Request
`GET`  `/profile/<profile_id>`
#### Response
```
307: Temporary Redirect
Location: http://<profile_module_ADDR>/?profile_endpoint?/<profile_id>

Redirecting to profile page for user: '<profile_id>' at http://<profile_module_ADDR>/?profile_endpoint?/<profile_id>
```

### Upload Product to Cart
#### Request
`POST` `/api/upload/<profile_id>`
```json
{
    "id": PRODUCT_ID_HERE,
    "sellerid": SELLER_ID_HERE,
    "name": "PRODUCT_NAME_HERE",
    "description": "PRODUCT_DESCRIPTION_HERE",
    "imgurl": "PRODUCT_IMAGE_HERE",
    "cost": PRODUCT_COST_HERE
}
```

### Remove Product from Cart
#### Request
`DELETE` `/api/remove/<profile_id>/<product_id>`



### Link to ad Module
#### Request
`GET`  `/cart/<userID>`



