// Define assembly-verified engine functions in the separate research project.
// @category HD2

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.CharDataType;
import ghidra.program.model.data.DataType;
import ghidra.program.model.data.FunctionDefinitionDataType;
import ghidra.program.model.data.IntegerDataType;
import ghidra.program.model.data.ParameterDefinition;
import ghidra.program.model.data.ParameterDefinitionImpl;
import ghidra.program.model.data.PointerDataType;
import ghidra.program.model.data.UnsignedIntegerDataType;
import ghidra.program.model.data.VoidDataType;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Function.FunctionUpdateType;
import ghidra.program.model.listing.Parameter;
import ghidra.program.model.listing.ParameterImpl;
import ghidra.program.model.listing.ReturnParameterImpl;
import ghidra.program.model.pcode.HighFunctionDBUtil;
import ghidra.program.model.symbol.SourceType;
import java.util.ArrayList;
import java.util.List;

public class ConfigureEngineModel extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (!"12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee"
                .equals(currentProgram.getExecutableSHA256())
                || currentProgram.getImageBase().getOffset() != 0x10000000L) {
            throw new IllegalArgumentException("Expected the original LS3DF.dll at its preferred base");
        }
        long[] entries = {
            0x100340b0L, 0x1005d260L, 0x10019f00L,
            0x10018e40L, 0x10068fb0L, 0x100151c0L, 0x1005d970L
        };
        for (long entry : entries) {
            Address address = toAddr(entry);
            Function function = getFunctionAt(address);
            if (function == null) {
                Function containing = getFunctionContaining(address);
                if (containing != null) {
                    throw new IllegalStateException(address + " overlaps " + containing.getEntryPoint());
                }
                if (!disassemble(address)) {
                    throw new IllegalStateException("Cannot disassemble verified entry " + address);
                }
                function = createFunction(address, null);
                if (function == null) {
                    throw new IllegalStateException("Cannot define verified entry " + address);
                }
            }
            println("Verified entry: " + function.getEntryPoint() + " " + function.getSignature());
        }
        Function open = getFunctionAt(toAddr(0x100340b0L));
        DataType pointer = new PointerDataType(VoidDataType.dataType);
        DataType[] types = {
            pointer, new PointerDataType(CharDataType.dataType),
            UnsignedIntegerDataType.dataType, pointer, pointer
        };
        String[] names = {"model", "resource_name", "flags", "opaque_arg4", "opaque_arg5"};
        List<Parameter> parameters = new ArrayList<>();
        for (int i = 0; i < names.length; i++) {
            parameters.add(new ParameterImpl(names[i], types[i], currentProgram));
        }
        open.updateFunction("__stdcall",
            new ReturnParameterImpl(IntegerDataType.dataType, currentProgram),
            parameters, FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS, true, SourceType.USER_DEFINED);
        FunctionDefinitionDataType reset = new FunctionDefinitionDataType("model_reset_slot");
        reset.setCallingConvention("__stdcall");
        reset.setReturnType(VoidDataType.dataType);
        reset.setArguments(new ParameterDefinition[] {
            new ParameterDefinitionImpl("model", pointer, null)
        });
        HighFunctionDBUtil.writeOverride(open, toAddr(0x100340bcL), reset);
        println("Configured model-open wrapper: " + open.getSignature());
    }
}
