
## Look at all non-virtual classes:
copClasses <- function(notYet = c("schlatherCopula")) {
    stopifnot(require("copula"))
    copcl <- unique(names(getClass("copula")@subclasses))
    isVirt <- vapply(copcl, isVirtualClass, NA)
    copcl <- copcl[!isVirt]
    copcl[notYet != copcl] #copcl[-match(notYet, copcl)]
}
copcl <- copClasses()

## TODO: Generalize to allow 'dim = 3'
## ----  ==> take only those which have a 'dim' argument
##' generates a list of copulas (dim = 2) from their class names
copObjs <- function(cl, first.arg = c("dim", "param"),
                    exclude = c("indepCopula", "lowfhCopula", "upfhCopula", "moCopula"), # don't have iTau(), for example.
                    envir = asNamespace("copula"))
{
    copF <- sapply(cl, get, envir=envir)
    frstArg <- vapply(copF, function(F) names(formals(F))[[1]], "")
    copF1 <- copF[frstArg %in% first.arg]
    objs <- lapply(copF1, function(.) .())
    stopifnot(sapply(objs, is, class2 = "copula"),
              sapply(objs, validObject))
    if(length(exclude)) objs[ !(names(objs) %in% exclude) ] else objs
}
copObs <- copObjs(copcl)
if(FALSE) ## including the indepCopula
    str(copObjs(copcl, exclude=NULL), max.level = 1)

copcl. <- names(copObs)# not including "indepCopula"

copO.2 <- copObs[excl.2 <- !(copcl. %in% c("amhCopula", "joeCopula", "tCopula"))]
                                        # because AMH has limited tau-range and t copula no iRho()
copBnds <- sapply(copObs, function(C)
                  c(min= C@param.lowbnd[1], max= C@param.upbnd[1]))
copBnd.2 <- copBnds[, excl.2]

if(FALSE) { ## inspect
    copcl
    str(copObs, max.level=1)
    ## The parameter bounds:
    t(copBnds)
}

