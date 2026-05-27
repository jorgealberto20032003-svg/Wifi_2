<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
{
    Schema::create('personal_medicos', function (Blueprint $table) {
        $table->id();
        $table->string('nombre');          // Nombre del médico/personal
        $table->integer('num_clinica');    // Número de consultorio o clínica
        $table->timestamps();
    });
}

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('personal_medicos');
    }
};
