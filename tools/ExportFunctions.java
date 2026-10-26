// Export explicitly selected functions and their references without program edits.
// @category HD2

import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.Symbol;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;

public class ExportFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2) {
            throw new IllegalArgumentException("Supply a new output directory and function addresses");
        }
        Path output = Path.of(args[0]);
        Files.createDirectory(output);
        DecompInterface decompiler = new DecompInterface();
        try {
            if (!decompiler.openProgram(currentProgram)) {
                throw new IllegalStateException(decompiler.getLastMessage());
            }
            for (int i = 1; i < args.length; i++) {
                monitor.checkCancelled();
                Address address = toAddr(args[i]);
                Function function = getFunctionAt(address);
                if (function == null) {
                    throw new IllegalArgumentException("No function starts at " + address);
                }
                JsonObject record = new JsonObject();
                record.addProperty("program", currentProgram.getName());
                record.addProperty("sha256", currentProgram.getExecutableSHA256());
                record.addProperty("entry", address.toString());
                record.addProperty("name", function.getName());
                record.addProperty("body_bytes", function.getBody().getNumAddresses());
                record.addProperty("signature", function.getSignature().toString());
                JsonArray callees = new JsonArray();
                for (Function callee : function.getCalledFunctions(monitor)) {
                    JsonObject call = new JsonObject();
                    call.addProperty("entry", callee.getEntryPoint().toString());
                    call.addProperty("name", callee.getName());
                    call.addProperty("external", callee.isExternal());
                    callees.add(call);
                }
                record.add("callees", callees);
                JsonArray callers = new JsonArray();
                for (Function caller : function.getCallingFunctions(monitor)) {
                    JsonObject call = new JsonObject();
                    call.addProperty("entry", caller.getEntryPoint().toString());
                    call.addProperty("name", caller.getName());
                    callers.add(call);
                }
                record.add("callers", callers);
                JsonArray references = new JsonArray();
                InstructionIterator instructions =
                    currentProgram.getListing().getInstructions(function.getBody(), true);
                while (instructions.hasNext()) {
                    Instruction instruction = instructions.next();
                    for (Reference reference : instruction.getReferencesFrom()) {
                        JsonObject ref = new JsonObject();
                        ref.addProperty("from", instruction.getAddress().toString());
                        ref.addProperty("instruction", instruction.toString());
                        ref.addProperty("to", reference.getToAddress().toString());
                        ref.addProperty("type", reference.getReferenceType().toString());
                        Symbol symbol = getSymbolAt(reference.getToAddress());
                        if (symbol != null) {
                            ref.addProperty("symbol", symbol.getName(true));
                        }
                        Data data = getDataAt(reference.getToAddress());
                        if (data != null && data.hasStringValue()) {
                            ref.addProperty("string", data.getValue().toString());
                        }
                        references.add(ref);
                    }
                }
                record.add("references", references);
                DecompileResults result = decompiler.decompileFunction(function, 45, monitor);
                record.addProperty("decompiled", result.decompileCompleted());
                record.addProperty("decompiler_message", result.getErrorMessage());
                if (result.decompileCompleted() && result.getDecompiledFunction() != null) {
                    Files.writeString(output.resolve(address + ".c"),
                        result.getDecompiledFunction().getC(), StandardCharsets.UTF_8,
                        StandardOpenOption.CREATE_NEW);
                }
                Files.writeString(output.resolve(address + ".json"),
                    new GsonBuilder().setPrettyPrinting().create().toJson(record),
                    StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW);
                println("Exported " + address + " " + function.getName() +
                    " decompiled=" + result.decompileCompleted());
            }
        }
        finally {
            decompiler.dispose();
        }
    }
}
