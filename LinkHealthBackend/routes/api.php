<?php

use Illuminate\Http\Request;
use Illuminate\Support\Facades\Route;
use App\Http\Controllers\Api\NetworkAccessController;

Route::post('/v1/validar-acceso', [NetworkAccessController::class, 'validar']);

Route::get('/user', function (Request $request) {
    return $request->user();
})->middleware('auth:sanctum');
