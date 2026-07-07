#pragma once
#include <cstdint>
namespace Hazel{
namespace Snap{
enum class Opcode:std::uint8_t{
    //NOTE:Any instruction that takes in immediate value cant be missing because that wont make sense
    /*
    We have 2 version of the VM. 
    One is the normal version which processes one data point at a time
    Other is the version for batch processing which processes multiple data points at a time. Basically each register stores an array so we need a for loop for each instruction.
    I think that will be more efficient than processing one data point at a time because we can do better cache locality and also we can use SIMD instructions for some of the instructions and compiler can auto vectorize some of the instructions. 
    SO both execute same bytecode but diffrently
    But this has a few issue worth keepin in mind.
    The first is that the function of batch processing will be larger than the normal version which can hurt cache locality.
    Another is that we have to implementation approach. 
    1)For every register we need to store an array. Scalar are not allowed i.e no broadcasting. This is good for most instruction. But for a lot of instruction 
      without immediate varient, we need to store the immediate value in a register as an array(The intermediate variant ones dont have such issue). 
      Which increases memory usage
    2)Add an if statment in the instruction to check if the register is scalar or array. If it is scalar then we can broadcast it to all the elements of the array. 
      This is good for memory usage but increases the size of the function and also increases the number of branches which can hurt performance.
    3)Try a hybrid approach. For some instruction we can use the first approach and for some instruction we can use the second approach. 

    I will go with the hybrid approach.
    */
    
    //How to understand the naming convention. Almost every opcode has one of type suffix. The type suffixes are of the <arg1>,<arg2>... format. arg<N> is the argument register/intermediate. Note that dest is optional. And if dest is given then the prefix of Opcode is the type of dest
    //I64 is a signed interger of 64 bits,I64FLOAT is a signed interger of 64 bits/fixed floating point and the instruction dont care, FLOAT is a fixed floating point number of 64 bits. The source of the operation is register i.e this references another variable
    //I64I is a signed interger of 64 bits,I64FLOATI is a signed interger of 64 bits/fixed floating point and the instruction dont care, FLOATI is a fixed floating point number of 64 bits. The source of the operation is an immediate value i.e this references a constant value
    //Note:An instruction with I<N> or II<N> expects all the argument to be of same sign. Like we dont care about the sign of argument but we expect all the arguments to be of same sign.
    //Note:An value can be missing. That is why conditional branch operation take 3 branches. And if missing then applying operation on missing value will return a missing value for most of the instruction(Exception being the operation designed to check if missing value or not)
    //     In the implementation for lets say add, we usually add the number and do an || operation on the bool of lhs.is_missing and rhs.is_missing. No need of if else. The value stored in missing valued register is not defined and accessing it is UB anyways
    //     But for stuff like SET, GET, DIV(Division by zero unintentionally when is missing is true), REM, we check few argument(Not all. Like for DIV only the b of a/b needs to be checked). Because that can cause segfault
    //We use fixed point floating point because faster and less weird behaviour. We have 4 fractional digits. So 1.2345 is stored as 12345. The scaling factor is 10000.
    /* ---- Loads -----------------------------------------
    * LOAD_* takes an immediate/register and a dest register. If dest and source type dont match then it is a cast.
    */
    OP_PTR_LOAD_PTR = 0,
    OP_I64_LOAD_I64, OP_I64_LOAD_FLOAT,
    OP_FLOAT_LOAD_I64, OP_FLOAT_LOAD_FLOAT,

    OP_PTR_LOAD_PTRI,
    OP_I64_LOAD_II64, OP_FLOAT_LOAD_FLOATI,

    
    /* ---- Binary Arithmetic Instructions ----------------------------------*/
    //We are using fixed point floating point. So if both lhs and rhs are of same type(Which they should be) then ADD,SUB,DIV,REM,MIN,MAX dont care if it is a float/integer
    //But multiplication care so we have 2 varients for that because of scaling. For FLOAT multiplication, we do in 128 bit space before scaling down
    //Div and rem return missing if rhs is 0 or (lhs,rhs) = (INT_MIN, -1) for signed integer. Because that is UB in C++.
    OP_I64FLOAT_ADD_I64FLOAT_I64FLOAT,
    OP_I64FLOAT_SUB_I64FLOAT_I64FLOAT,
    OP_I64_MUL_I64_I64, OP_FLOAT_MUL_TRUNC_FLOAT_FLOAT, OP_FLOAT_MUL_ROUND_FLOAT_FLOAT,//The trunc and round are 2 diffrent mode from which user can choose
    OP_I64FLOAT_DIV_I64FLOAT_I64FLOAT,
    OP_I64FLOAT_REM_I64FLOAT_I64FLOAT,
    OP_I64FLOAT_MIN_I64FLOAT_I64FLOAT,
    OP_I64FLOAT_MAX_I64FLOAT_I64FLOAT,

    //Add,mul,min,max is commutative so we can have immediate as second argument and that will be enough
    OP_I64FLOAT_ADD_I64FLOAT_I64FLOATI,
    OP_I64_MUL_I64_II64, OP_FLOAT_MUL_TRUNC_FLOAT_FLOATI, OP_FLOAT_MUL_ROUND_FLOAT_FLOATI,
    OP_I64FLOAT_DIV_I64FLOAT_I64FLOATI,
    OP_I64FLOAT_REM_I64FLOAT_I64FLOATI,
    OP_I64FLOAT_MAX_I64FLOAT_I64FLOATI,
    OP_I64FLOAT_MIN_I64FLOAT_I64FLOATI,

    OP_I64FLOAT_SUB_I64FLOATI_I64FLOAT,//No varient of subtraction with immediate as second argument because sub(a,b) = add(a,-b) 
    OP_I64FLOAT_DIV_I64FLOATI_I64FLOAT,
    OP_I64FLOAT_REM_I64FLOATI_I64FLOAT,


    /* ---- Trinary Arithmetic Instructions ------- */
    //The following instructon are important for general purpose programming. But I doubt of much use for this specific use case. Removing it for now. If we need we can add it back.
    // OP_I64_FMA_I64_I64_I64, OP_FLOAT_FMA_FLOAT_FLOAT_FLOAT,
    //The following are removed because they are rarer and can be implemented using the above instruction + one SUB instruction
    // OP_I64_FMS_I64_I64_I64, OP_FLOAT_FMS_FLOAT_FLOAT_FLOAT,
    // OP_I64_FNMA_I64_I64_I64, OP_FLOAT_FNMA_FLOAT_FLOAT_FLOAT,
    // OP_I64_FNMS_I64_I64_I64, OP_FLOAT_FNMS_FLOAT_FLOAT_FLOAT,

    // OP_I64_FMA_II64_I64_I64, OP_FLOAT_FMA_FLOATI_FLOAT_FLOAT,
    // OP_I64_FMA_II64_I64_II64, OP_FLOAT_FMA_FLOATI_FLOAT_FLOATI,
    // OP_I64_FMA_I64_I64_II64, OP_FLOAT_FMA_FLOAT_FLOAT_FLOATI,
    //Same reason as above
    // OP_I64_FMS_II64_I64_II64, OP_FLOAT_FMS_FLOATI_FLOAT_FLOATI,
    // OP_I64_FNMA_I64_I64_II64, OP_FLOAT_FNMA_FLOAT_FLOAT_FLOATI,


    /* ---- Other Arithmetic Instructions ------- */
    OP_I64_WEIGHTED_ADD_II64_I64_REG_WEIGHT, OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_REG_WEIGHT, OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_REG_WEIGHT, 
    OP_I64_WEIGHTED_ADD_II64_II64_REG_WEIGHT, OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_REG_WEIGHT, OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_REG_WEIGHT, 
    OP_I64_WEIGHTED_ADD_II64_I64_IMM_WEIGHT, OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOAT_IMM_WEIGHT, OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOAT_IMM_WEIGHT, 
    OP_I64_WEIGHTED_ADD_II64_II64_IMM_WEIGHT, OP_FLOAT_WEIGHTED_ADD_TRUNC_II64_FLOATI_IMM_WEIGHT, OP_FLOAT_WEIGHTED_ADD_ROUND_II64_FLOATI_IMM_WEIGHT, 

    /* ---- Binary Bitwise Instructions ----------- */
    OP_I64_BIT_AND_I64_I64, OP_I64_BIT_OR_I64_I64, OP_I64_BIT_XOR_I64_I64,
    // OP_I64_SHL_I64_I64, OP_I64_ASHR_I64_I64, OP_I64_LSHR_I64_I64,
    // OP_I64_ROTL_I64_I64, OP_I64_ROTR_I64_I64, //Rare

    // Bitwise instruction are rare for our use case so immediate variants are removed. They can be implemented using the above instruction + one LOAD instruction
    // OP_I64_BIT_AND_I64_II64, OP_I64_BIT_OR_I64_II64,
    // OP_I64_SHL_I64_II64, OP_I64_ASHR_I64_II64, OP_I64_LSHR_I64_II64,
    // OP_I64_ROTL_I64_II64, OP_I64_ROTR_I64_II64,

    // OP_I64_SHL_II64_I64, OP_I64_ASHR_II64_I64, OP_I64_LSHR_II64_I64,
    // OP_I64_ROTL_II64_I64, OP_I64_ROTR_II64_I64,
    OP_I64_BIT_XOR_I64_II64,//Why only this one? Because we need not instruction. We can either use register-register XOR(2 instruction) or a seperate NOT instruction
                               //or introduce a seperate register-immediate XOR instruction(1 instruction). 
                               //The latter is has same efficiency + more flexibility 
 

    /* ---- Comparison Instruction ------------------*/
    //We are using fixed point floating point. So if both lhs and rhs are of same type(Which they should be) then comparison instructions dont care if it is a float/integer
    OP_I64_EQ_PTRI64FLOAT_PTRI64FLOAT,
    OP_I64_NEQ_PTRI64FLOAT_PTRI64FLOAT,
    OP_I64_GT_PTRI64FLOAT_PTRI64FLOAT,
    OP_I64_GE_PTRI64FLOAT_PTRI64FLOAT,
    OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT,//(lower,upper,step)
    OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT,//(lower,upper,step)
    OP_I64_IN_RANGE_PTR_PTR_PTR_I64,//(lower,upper,step) Note:-The step in bytes is scale*step. Where scale is sizeof element
    OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64,//(lower,upper,step) Note:-The step in bytes is scale*step. Where scale is sizeof element

    OP_I64_EQ_PTRI64FLOAT_PTRI64FLOATI,
    OP_I64_NEQ_PTRI64FLOAT_PTRI64FLOATI,
    OP_I64_GT_PTRI64FLOAT_PTRI64FLOATI,
    OP_I64_GE_PTRI64FLOAT_PTRI64FLOATI,
    OP_I64_GT_PTRI64FLOATI_PTRI64FLOAT,
    OP_I64_GE_PTRI64FLOATI_PTRI64FLOAT,
    OP_I64_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI,//Usually step is a constant
    OP_I64_NOT_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI,//Usually step is a constant
    OP_I64_IN_RANGE_PTR_PTR_PTR_I64I,//(lower,upper,step) Note:-The step in bytes is scale*step. Where scale is sizeof element
    OP_I64_NOT_IN_RANGE_PTR_PTR_PTR_I64I,//(lower,upper,step) Note:-The step in bytes is scale*step. Where scale is sizeof element
    //I can expand more for IN_RANGE but the explosion of instruction is not worth it. 

 
    /* ---- Unary Instruction ------------------*/
    OP_I64FLOAT_ABS_I64FLOAT,
    OP_FLOAT_CEIL_FLOAT,
    OP_FLOAT_FLOOR_FLOAT,
    OP_FLOAT_INTEGRAL_PART_FLOAT,
    OP_FLOAT_FRACTIONAL_PART_FLOAT,
    OP_FLOAT_ROUNDNEAREST_FLOAT,
    OP_FLOAT_ROUNDEVEN_FLOAT,
    OP_FLOAT_SQRT_FLOAT,
    OP_I64_HAS_VALUE,//Expects a register. Checks if the value stored in register has value or not
    OP_I64_HAS_NO_VALUE,//Expects a register. Checks if the value stored in register has value or not
    

    /* ---- Fused Compare-and-Branch(Takes in 2 or 4 operands + 3 branches(if branch,else branch and data missing branch)) ----------------------------------------*/
    OP_BR_EQ_PTRI64FLOAT_PTRI64FLOAT,
    OP_BR_GT_PTRI64FLOAT_PTRI64FLOAT,
    OP_BR_OR_I64_I64,
    OP_BR_AND_I64_I64,
    OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOAT,
    OP_BR_IN_RANGE_PTR_PTR_PTR_I64,
    
    OP_BR_EQ_I64FLOAT_I64FLOATI,
    OP_BR_GT_I64FLOAT_I64FLOATI,
    OP_BR_GT_I64FLOATI_I64FLOAT,
    OP_BR_IN_RANGE_I64FLOAT_I64FLOAT_I64FLOAT_I64FLOATI,
    OP_BR_IN_RANGE_PTR_PTR_PTR_I64I,
 

    /*--------Regular branch-----*/
    OP_BR,//Only takes in one branch. Unconditional
    OP_BR_TRUE,//Only takes in 1 condition operand + 3 branches. 


    /* ---- Calls --------------------------------------- */
    OP_CALL,
    OP_EXTERN_CALL_PTR,//Takes in a pointer(immediate pointer) to the function. The ptr can be from DL_OPEN or injected by the compiler
    OP_RET,//Can return maximum of 16 values at once
 
    
    /* ---- Heap objects: arrays ---------------------------------------------*/
    //ALLOC,REALLOC,FREE is managed by the GC or just malloc/free
    OP_PTR_ALLOC_I64_I64_II64, //First parameter is the length(i.e non empty),second parameter is the capacity,third parameter is size of each element in bytes(No of element allocated is capacity*element size)
    OP_PTR_REALLOC_I64_I64_II64, //First parameter is the new length, Second parameter is the new capacity,Third parameter is size of each element(No of element allocated is capacity*element size)
    OP_FREE_PTR, 

    OP_I64_GET_PTR_I64,//dest = *(a+b*scale) (bounds-checked). If offset < 0 then it losses this bound check
    OP_FLOAT_GET_PTR_I64,//dest = *(a+b*scale) (bounds-checked). If offset < 0 then it losses this bound check
    OP_PTR_GET_PTR_I64,//dest = *(a+b*scale) (bounds-checked). If offset < 0 then it losses this bound check

    OP_I64_GET_PTR_II64,//dest = *(a+b*scale) (bounds-checked). If offset < 0 then it losses this bound check
    OP_FLOAT_GET_PTR_II64,//dest = *(a+b*scale) (bounds-checked). If offset < 0 then it losses this bound check
    OP_PTR_GET_PTR_II64,//dest = *(a+b*scale) (bounds-checked). If offset < 0 then it losses this bound check

    OP_PTR_PTROFFSET_ADD_PTR_I64,//dest = a + b*scale (pointer arithmetic, b is number of element. Bound checked. Capacity set to 0 if out of range). If offset < 0 then it losses this bound check
    OP_PTR_PTROFFSET_ADD_PTR_II64,//dest = a + b*scale (pointer arithmetic, b is number of element. Bound checked. Capacity set to 0 if out of range). If offset < 0 then it losses this bound check
    OP_PTR_PTROFFSET_SUB_PTR_I64,//dest = a - b*scale (pointer arithmetic, b is number of element. Bound checked. Capacity set to 0 if out of range). If offset < 0 then it losses this bound check

    //If missing or out of range then we do nothing
    OP_SET_PTR_I64_I64FLOAT,//*(a+b*scale) = c. The second item is the offset in bytes. Both float and int are 64 bits and SET basically writes those bits. SO dont matter
    OP_SET_PTR_I64_PTR,//*(a+b*scale) = c. The second item is the offset in bytes

    OP_SET_PTR_II64_I64FLOAT,//*(a+b*scale) = c. The second item is the offset in bytes
    OP_SET_PTR_II64_PTR,//*(a+b*scale) = c. The second item is the offset in bytes

    OP_I64_LEN_PTR,//dest = length(a)
    OP_I64_CAP_PTR,//dest = capacity(a)
    OP_I64_ELM_SIZE_PTR,//dest = size of each element in bytes
    //For push and pop, if the pointer is from pointer offset then that is treated as a new array and the length and capacity starts from the remaining length and capacity.
    //SO keep it in mind cuz push or pop on the pointer offset will not change the length and capacity of the original array. 
    //It will only change the length and capacity of the pointer offset array
    OP_POP_PTR_I64,//POP the number of elements specified by the second argument
    OP_PUSH_PTR_PTR_I64,//PUSH the number of elements specified by the second argument
    OP_PUSH_PTR_I64FLOAT,//PUSH 164 or FLOAT value to the end of the array. The second argument is the value to be pushed
    OP_MEMCPY_PTR_PTR_I64,//dest = memcpy(a,b,c) where c is the number of bytes to copy
    OP_MEMMOVE_PTR_PTR_I64,//dest = memmove(a,b,c) where c is the number of bytes to copy
    OP_I64_UNSIGNED_CMP_PTR_PTR_I64,//dest = memcmp(a,b,c) where c is the number of bytes to compare. Either negative, zero or positive value is returned based on the comparison
    
    /* ---- FFI --------------------------------------------------*/
    // No FFI for safety reasons. The compiler must provide all the external function needed via ptr but no run time stuff via DL open
    // OP_PTR_DL_OPEN_PTR,//dest = dlopen/LoadLibrary(Ptr to string)
    // OP_DL_CLOSE_PTR,
    // OP_PTR_DL_GET_FUNC_PTR_PTR,//Takens in a string ptr and the ptr to the library u get from DL_OPEN. Returns the function pointer
 

    /*----Time series related--------------------------------------
     *These are not strictly related to time series but will be used there most*/
    OP_I64_BINARY_FIND_GE_PTR_I64_I64FLOAT,  // index of first entry with value >= x (or "Missing" if no such entry exists). The second argument is the index at which to start
    OP_I64_BINARY_FIND_LE_PTR_I64_I64FLOAT,  // index of last entry with value <= x (or "Missing" if no such entry exists). The second argument is the index at which to start
    OP_I64_BINARY_FIND_GE_PTR_II64_I64FLOAT,//The index is often 0 so we allow them to just pass an immediate value with this
    OP_I64_BINARY_FIND_LE_PTR_II64_I64FLOAT,//The index is often 0 so we allow them to just pass an immediate value with thisk

    /*-------Traceback--------------------------------*/
    OP_PRINT_PTR,//Prints the string pointed by the pointer
    //Nothing else because how a language wants to do traceback is too diffrent to provide. It is better to let the language implement it themselves vis FFI
    //And in case a constraint is violated then the language is supposed to throw and exit anyways so if that process is slow then not a big of a deal
  
    /* ---- No-ops / control -------------------------------------------- */
    OP_NOP,//Does nothing, useful for padding or reserved space
    OP_HALT,// stop execution, return to host. Must be last instruction in the program. Returns -1 to host as a sign of successful execution
    OP_TRAP,// stop execution, return to host and give index at with trap occurred. Helps with creating a traceback
    OP_IF_TRUE_TRAP,// stop execution, return to host and give index at with trap occurred if the value in the register is true. Helps with creating a traceback. Also traps if the condition is missing
    OP_IF_MISSING_TRAP,// stop execution, return to host and give index at with trap occurred if the value in the register is missing. Helps with creating a traceback 
    OP_TRACE,// Dont stop execution but add the pc of this instruction to the path vector. Helps with creating a traceback

    OP_OPCODE_COUNT    /* keep last: total opcode count, sizes dispatch tables */
};
namespace Trend{};
};
};