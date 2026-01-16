Lets strt working on parsing attributes by adding them to the grammar as `Phase 5.0`
- Please follow the document flow
- Attribute node has already been added
- Attributes have the following format
```
@attr
@attr(1) // all arguments are literals
@attr({a: 1}) // named arguments, all of them are literals
@[attr1, attr1(10), attr3({a: 20})] // multiple arguments
```
