# not well behaved: redefinition of library function

outer <- function(a){
x <- a 
z <- (function() { x+1})()
z
}
outer(6)
