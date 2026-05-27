<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class PersonalMedico extends Model
{
    protected $table = 'personal_medicos';

    protected $fillable = [
        'nombre',
        'num_clinica',
    ];
}