<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use Illuminate\Http\Request;
use App\Models\PersonalMedico;

class NetworkAccessController extends Controller
{
    public function validar(Request $request)
    {
        $request->validate([
            'nombre' => 'required|string',
            'clinica' => 'required|integer',
        ]);

        // Busca coincidencia exacta en la base de datos de XAMPP
        $usuarioExiste = PersonalMedico::where('nombre', 'LIKE', trim($request->nombre))
                                      ->where('num_clinica', $request->clinica)
                                      ->exists();

        return response()->json([
            'autorizado' => $usuarioExiste
        ], 200);
    }
}