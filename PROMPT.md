```
pub class Demo {
  `Annotation = i32
  value i32 // field declaration with type
  value2 = T()  // field declaration with default
  priv value3 T = T() // field declaration will access modifier
  
  func say() {} 
  const func say() {}
  priv func done() {}
  @inline
  func who(){}
  func `+`(x i32) => x + 10 // Binary operator overload
  func `()`() {} // Call operator overload
  func `[]`(x i32) {} // Index operator overload
  func `[]=`(key string, value i32){} // Index assignment operator overload
  func `as i32`() {} // Cast operator overload
  func `&.`() {} // Redirect. if it returns a.b.c to be writtern as a&.c if the overload returns b
  func `..`() {} // range operator overload
}

// Structs & classes can have base, semantics can be different
@poco
struct Hello : Base {
}

class Print<T, U: isInteger, V = i32>: Base {
}
```
