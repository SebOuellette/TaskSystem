# Task System

This is a custom task system webpage

## Documentation

### How to use
To compile, you must be within a docker container. To first pull the docker container to update/install it onto your host system, use the following command: `docker pull sebouellette/webserver:shoppingcart`

To enter the docker container, find startup.txt in the Shared folder. This file contains the commands to start the docker instance on different types of terminals. 

Once you are within the docker container terminal, enter the `/Shared` directory using the `cd` command. Here, you can run the `./recompile.sh` command to recompile the project. The executable will be placed in the newly created `build` folder. If a build folder already exists, it will be deleted and a new one will be created while compiling. 

If you only wish to compile without clearing out the current build, you can `cd build`, then use `make` to compile changes quickly. 

To use the compiled executable, simply start it using the following comamnd `/Shared/build/tasksystem`.

### Legend
`<hello>` -- Text surrounded by <> is a variable. Do not include <> when using the API.<br>
`?foobar?` -- Text surrounded by ?? is content that is to be decided later in development. 


### View home page
### Request
`GET` `/`





