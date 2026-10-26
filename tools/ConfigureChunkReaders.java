// Apply assembly-supported archive/chunk reader signatures in the research project.
// @category HD2

import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.BooleanDataType;
import ghidra.program.model.data.DataType;
import ghidra.program.model.data.IntegerDataType;
import ghidra.program.model.data.PointerDataType;
import ghidra.program.model.data.UnsignedIntegerDataType;
import ghidra.program.model.data.UnsignedShortDataType;
import ghidra.program.model.data.VoidDataType;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Function.FunctionUpdateType;
import ghidra.program.model.listing.Parameter;
import ghidra.program.model.listing.ParameterImpl;
import ghidra.program.model.listing.ReturnParameterImpl;
import ghidra.program.model.symbol.SourceType;
import java.util.ArrayList;
import java.util.List;

public class ConfigureChunkReaders extends GhidraScript {
    private Function requireFunction(long address) {
        Function function = getFunctionAt(toAddr(address));
        if (function == null) {
            throw new IllegalStateException("Missing function at " + toAddr(address));
        }
        return function;
    }

    private Function requireImport(long address, String name) {
        Function thunk = requireFunction(address);
        if (!thunk.isThunk()) {
            throw new IllegalStateException("Expected import thunk at " + thunk.getEntryPoint());
        }
        Function target = thunk.getThunkedFunction(true);
        if (!target.isExternal() || !target.getName().equals(name)) {
            throw new IllegalStateException("Unexpected import target at " + thunk.getEntryPoint());
        }
        return target;
    }

    private void configure(Function function, String convention, DataType result,
            String[] names, DataType... types) throws Exception {
        List<Parameter> parameters = new ArrayList<>();
        for (int i = 0; i < names.length; i++) {
            parameters.add(new ParameterImpl(names[i], types[i], currentProgram));
        }
        println("Before: " + function.getSignature());
        function.updateFunction(convention, new ReturnParameterImpl(result, currentProgram),
            parameters, FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS, true, SourceType.USER_DEFINED);
        println("After: " + function.getSignature());
    }

    @Override
    public void run() throws Exception {
        if (!"8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307"
                .equals(currentProgram.getExecutableSHA256())) {
            throw new IllegalArgumentException("This script is specific to the unpacked base client");
        }
        Function seek = requireImport(0x7e4f38L, "dtaSeek");
        Function read = requireImport(0x7e4f3eL, "dtaRead");
        Function write = requireImport(0x7e4f32L, "dtaWrite");
        Function enter = requireFunction(0x415c90L);
        Function boundedRead = requireFunction(0x405940L);
        Function leave = requireFunction(0x405890L);
        if (enter.getStackPurgeSize() != 4 || boundedRead.getStackPurgeSize() != 8
                || leave.getStackPurgeSize() != 4) {
            throw new IllegalStateException("Unexpected chunk-reader stack cleanup");
        }
        DataType integer = IntegerDataType.dataType;
        DataType unsigned = UnsignedIntegerDataType.dataType;
        DataType pointer = new PointerDataType(VoidDataType.dataType);
        configure(seek, "__stdcall", integer,
            new String[] {"handle", "offset", "origin"}, unsigned, integer, integer);
        configure(read, "__stdcall", integer,
            new String[] {"handle", "buffer", "count"}, unsigned, pointer, unsigned);
        configure(write, "__stdcall", integer,
            new String[] {"handle", "buffer", "count"}, unsigned, pointer, unsigned);
        configure(enter, "__thiscall", BooleanDataType.dataType,
            new String[] {"chunk_id"}, new PointerDataType(UnsignedShortDataType.dataType));
        configure(boundedRead, "__thiscall", BooleanDataType.dataType,
            new String[] {"buffer", "count"}, pointer, unsigned);
        configure(leave, "__thiscall", BooleanDataType.dataType,
            new String[] {"written_size"}, new PointerDataType(unsigned));
    }
}
